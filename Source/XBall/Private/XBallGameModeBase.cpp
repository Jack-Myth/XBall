// Fill out your copyright notice in the Description page of Project Settings.

#include "XBallGameModeBase.h"
#include "MyBPFuncLib.h"
#include "CircleOnlyMapGenerator.h"
#include "XBallPlayerControllerBase.h"
#include "Kismet/GameplayStatics.h"
#include "XBallBase.h"
#include "Engine/World.h"
#include "XBallPlayerState.h"

AXBallGameModeBase::AXBallGameModeBase()
{
	PlayerControllerClass = AXBallPlayerControllerBase::StaticClass();
	DefaultPawnClass = AXBallBase::StaticClass();
	PlayerStateClass = AXBallPlayerState::StaticClass();
}

void AXBallGameModeBase::BeginPlay()
{
	Super::BeginPlay();
}

void AXBallGameModeBase::SetIsTeamGame(bool isTeamGame)
{
	bIsTeamGame = isTeamGame;
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
			//StartNewPlayer(it->TargetController);
			it.RemoveCurrent();
		}
	}
}

void AXBallGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	if (!MapSeed)
	{
		do
		{
			MapSeed = FMath::Rand();
		} while (!MapSeed);
	}
	TArray<FBlockInfo> x = UMyBPFuncLib::CollectMapModified(this);
	GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Green, FString::Printf(TEXT("Seed Is:%d"), MapSeed));
	((AXBallPlayerControllerBase*)NewPlayer)->ReGenOldMap(ACircleOnlyMapGenerator::StaticClass(), 100, 100, 50, MapSeed,x);
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
	}
	else
	{
		PlayerRespawnQueue[PlayerIndex].TimeRemain = 5;
	}
}

void AXBallGameModeBase::RestartPlayerAtTransform(AController* NewPlayer, const FTransform& SpawnTransform)
{
	TArray<AActor*> ActorsCollection;
	FTransform tmpTransform = SpawnTransform;
	TArray<TEnumAsByte<EObjectTypeQuery>> tmpx;
	while (UKismetSystemLibrary::SphereOverlapActors(this, tmpTransform.GetLocation(), 100.f, tmpx, AActor::StaticClass(), TArray<AActor*>(), ActorsCollection))
	{
		tmpTransform.SetLocation(FVector(FMath::RandRange(-500, 500), FMath::RandRange(-500, 500), FMath::RandRange(-500, 500)));
	}
	Super::RestartPlayerAtTransform(NewPlayer, tmpTransform);
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