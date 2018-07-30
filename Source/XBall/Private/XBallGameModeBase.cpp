// Fill out your copyright notice in the Description page of Project Settings.

#include "XBallGameModeBase.h"
#include "MyBPFuncLib.h"
#include "CircleOnlyMapGenerator.h"
#include "XBallPlayerControllerBase.h"
#include "Kismet/GameplayStatics.h"
#include "XBallBase.h"
#include "Engine/World.h"
#include "XBallPlayerState.h"
#include "XBallGameInstanceNative.h"
#include "ConstructorHelpers.h"
#include "XBallGameSession.h"
#include "SimplexNoiseBPLibrary.h"

AXBallGameModeBase::AXBallGameModeBase()
{
	PlayerControllerClass = AXBallPlayerControllerBase::StaticClass();
	DefaultPawnClass = AXBallBase::StaticClass();
	PlayerStateClass = AXBallPlayerState::StaticClass();
	GameSessionClass = AXBallGameSession::StaticClass();
}

void AXBallGameModeBase::BeginPlay()
{
	Super::BeginPlay();
}

void AXBallGameModeBase::SetIsTeamGame(bool isTeamGame)
{
	bIsTeamGame = isTeamGame;
	//Sync To Controller
	TArray<AActor*> Controllers;
	UGameplayStatics::GetAllActorsOfClass(this, AXBallPlayerControllerBase::StaticClass(), Controllers);
	for (int i=0;i<Controllers.Num();i++)
	{
		Cast<AXBallPlayerControllerBase>(Controllers[i])->bIsTeamPlay = isTeamGame;
	}
}

bool AXBallGameModeBase::IsTeamGame()
{
	return bIsTeamGame;
}

void AXBallGameModeBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	for (auto it=PlayerRespawnQueue.CreateIterator();it;++it)
	{
		it->TimeRemain -= DeltaSeconds;
		if (it->TimeRemain<=0)
		{
			HandleStartingNewPlayer(it->TargetController);
			it->TargetController->CloseWaitingRespawn();
			//StartNewPlayer(it->TargetController);
			it.RemoveCurrent();
		}
	}
}

void AXBallGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	//It Used for some test situations.
	//Such as load map directly(not travel from Lobby).
	if (!MapSeed)
	{
		do
		{
			MapSeed = FMath::Rand();
		} while (!MapSeed);
	}
	//For the player that join game after GameOver.
	//ShowGameResult.
	if (IsGameOver)
	{
		AXBallPlayerControllerBase* XBallController = Cast<AXBallPlayerControllerBase>(NewPlayer);
		if (XBallController)
		{
			XBallController->ShowResultSync(0);
			XBallController->SetIsInLobby(true);
		}
	}
	InitPlayerController(NewPlayer);
}

void AXBallGameModeBase::PrepareReSpawn(AXBallPlayerControllerBase* TargetController)
{
	int PlayerIndex= PlayerRespawnQueue.FindLastByPredicate([TargetController](const FPlayerRespawnInfo respawnInfo)
		{
			if (respawnInfo.TargetController == TargetController)
				return true;
			return false;	
		});
	if (PlayerIndex==INDEX_NONE)
	{
		FPlayerRespawnInfo tmpInfo;
		tmpInfo.TargetController = TargetController;
		tmpInfo.TimeRemain = 5;
		PlayerRespawnQueue.Add(tmpInfo);
		TargetController->ShowWaitingRespawn();
	}
	else
	{
		PlayerRespawnQueue[PlayerIndex].TimeRemain = 5;
	}
}

UClass* AXBallGameModeBase::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	AXBallPlayerControllerBase* XballController = Cast<AXBallPlayerControllerBase>(InController);
	if (XballController)
	{
		UClass* DefaultXBallClass= XballController->GetPlayerDefaultCharacter();
		return DefaultXBallClass ? DefaultXBallClass : DefaultPawnClass;
	}
	else
	{
		return DefaultPawnClass;
	}
}

void AXBallGameModeBase::RestartPlayerAtTransform(AController* NewPlayer, const FTransform& SpawnTransform)
{
	TArray<AActor*> ActorsCollection;
	FTransform tmpTransform = SpawnTransform;
	FVector SpawnLocation;
	do 
	{
		SpawnLocation = FVector(FMath::RandRange(-2000, 2000), FMath::RandRange(-2000, 2000), FMath::RandRange(-2000, 2000));
	} while (USimplexNoiseBPLibrary::SimplexNoise3D(SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z) > 0.5);
	tmpTransform.SetLocation(SpawnLocation);
	Super::RestartPlayerAtTransform(NewPlayer, tmpTransform);
}

void AXBallGameModeBase::CheckScore()
{
	if (GetMatchState() == "Ended")
		return;
	TArray<AActor*> XBallStates;
	UGameplayStatics::GetAllActorsOfClass(this, AXBallPlayerState::StaticClass(), XBallStates);
	if (XBallStates.Num())
	{
		if (IsTeamGame())
		{
			TArray<int> TeamsScore;
			TeamsScore.SetNum(5, false);
			for (AActor*& XBallStateActor:XBallStates)
			{
				AXBallPlayerState* XBallState = Cast<AXBallPlayerState>(XBallStateActor);
				if (XBallState->Team>0&&XBallState->Team<5)
				{
					TeamsScore[XBallState->Team] += XBallState->KillScore;
				}
			}
			for (int i=0;i<TeamsScore.Num();i++)
			{
				if (TeamsScore[i]>=TargetScore)
				{
					SetMatchState("Ended");
					RiseGameOver(i);
					return;
				}
			}
		}
		else
		{
			for (AActor*& XBallStateActor : XBallStates)
			{
				AXBallPlayerState* XBallState = Cast<AXBallPlayerState>(XBallStateActor);
				if (XBallState->KillScore>=TargetScore)
				{
					RiseGameOver(0);
					return;
				}
			}
		}
	}
}

void AXBallGameModeBase::RiseGameOver(int WinTeam)
{
	IsGameOver = true;
	TArray<AActor*> XBallControllers;
	UGameplayStatics::GetAllActorsOfClass(this, AXBallPlayerControllerBase::StaticClass(), XBallControllers);
	for (AActor*& XBallController:XBallControllers)
	{
		Cast<AXBallPlayerControllerBase>(XBallController)->ShowResultSync(WinTeam);
		Cast<AXBallPlayerControllerBase>(XBallController)->SetIsInLobby(true);
	}
	if (IsRunningDedicatedServer())
	{
		FTimerHandle tmpTimerH;
		GetWorld()->GetTimerManager().SetTimer(tmpTimerH, [=]()
			{
				RestartGame();
			}, 15, false, 15);
	}
}
void AXBallGameModeBase::InitXBallGame(bool IsTeamPlay, int TargetScore, int InitMoney, int MaxE/*=100*/, int MaxW/*=100*/, int MaxH/*=50*/)
{
	GEngine->AddOnScreenDebugMessage(-1, 20, FColor::Green, "Init XBall Game!");
	SetIsTeamGame(IsTeamPlay);
	this->TargetScore = TargetScore;
	this->InitMoney = InitMoney;
	this->MaxE = MaxE;
	this->MaxW = MaxW;
	this->MaxH = MaxH;
	//Gen Map Seed
	if (!MapSeed)
	{
		do
		{
			MapSeed = FMath::Rand();
		} while (!MapSeed);
	}
	//For Dedicated Server, the serverside didn't have Local playercontroller;
	//So It need to build map in InitXBallGame()
	if (IsRunningDedicatedServer())
	{
		UMyBPFuncLib::GenWorld(this, MaxE, MaxW, MaxH, MapSeed);
	}
	//Notify Existing Controller
	for (auto it = GetWorld()->GetPlayerControllerIterator(); it; ++it)
	{
		//Check If Client has complietly loaded the map
		//The other controller will be handle by 
		if ((*it)->HasClientLoadedCurrentWorld())
		{
			InitPlayerController(it->Get());
		}
	}
}

void AXBallGameModeBase::InitPlayerController(APlayerController* PlayerControllerToInit)
{
	//Ignore Init Request when Host doesn't completely loaded map.
	if (GetWorld()->IsInSeamlessTravel())
		return;
	GEngine->AddOnScreenDebugMessage(-1, 20, FColor::Green, "Init PlayerController!");
	AXBallPlayerControllerBase* NewPlayer = Cast<AXBallPlayerControllerBase>(PlayerControllerToInit);
	if (!NewPlayer)
		return;
	TArray<FBlockInfo> x = UMyBPFuncLib::CollectMapModified(this);
	NewPlayer->ReGenOldMap(ACircleOnlyMapGenerator::StaticClass(), MaxE, MaxW, MaxH, MapSeed, x);
	NewPlayer->InitGameUI();
	NewPlayer->SetIsInLobby(false);
	NewPlayer->Coins = InitMoney;
	NewPlayer->bIsTeamPlay = IsTeamGame();
}

void AXBallGameModeBase::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();
	//Get Stored Value From GameInstance
	UXBallGameInstanceNative* GameInstance = Cast<UXBallGameInstanceNative>(UGameplayStatics::GetGameInstance(this));
	if (GameInstance)
	{
		InitXBallGame(GameInstance->StoredFloat.FindRef("bIsTeamPlay"),
			GameInstance->StoredFloat.FindRef("TargetScore"),
			GameInstance->StoredFloat.FindRef("InitMoney"),
			GameInstance->StoredFloat.FindRef("MaxE"),
			GameInstance->StoredFloat.FindRef("MaxW"),
			GameInstance->StoredFloat.FindRef("MaxH"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed To Cast GameInstance!"));
	}
}

void AXBallGameModeBase::Logout(AController* Exiting)
{
	TArray<AActor*> Controllers;
	UGameplayStatics::GetAllActorsOfClass(this, AXBallPlayerControllerBase::StaticClass(), Controllers);
	if (!Controllers.Num()||(Controllers.Num()==1&&Controllers[0]==Exiting))
	{
		//Clear All Map
		//Release Mem resource.
		UMyBPFuncLib::ClearMap(this);
		//Regen map for next match.
		UMyBPFuncLib::GenWorld(this, MaxE, MaxW, MaxH, MapSeed);
	}
}

void AXBallGameModeBase::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
	UE_LOG(LogTemp, Display, TEXT("Match Has Started"));
}

void AXBallGameModeBase::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();
	UE_LOG(LogTemp, Display, TEXT("Match Has Ended"));
}

bool AXBallGameModeBase::IsMatchInProgress() const
{
	UE_LOG(LogTemp, Display, TEXT("Is Match In Progress"));
	return true;
}

/*APawn* AXBallGameModeBase::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform)
{	
	return Super::SpawnDefaultPawnAtTransform_Implementation(NewPlayer, SpawnTransform);
	AXBallPlayerControllerBase* NewXBallPlayer = Cast<AXBallPlayerControllerBase>(NewPlayer);
	if (!NewXBallPlayer)
	{
		return Super::SpawnDefaultPawnAtTransform_Implementation(NewPlayer, SpawnTransform);
	}
	GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Cyan, FString("Character Pawn Class:") + NewXBallPlayer->GetPlayerDefaultCharacter()->GetFullName());
	return Cast<APawn>(GetWorld()->SpawnActor(NewXBallPlayer->GetPlayerDefaultCharacter(), &SpawnTransform));
}
*/