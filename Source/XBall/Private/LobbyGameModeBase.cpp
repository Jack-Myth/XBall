// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyGameModeBase.h"
#include "Engine/World.h"
#include "UserWidget.h"
#include "WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "XBallGameModeBase.h"
#include "XBallGameInstanceNative.h"
#include "XBallPlayerControllerBase.h"
#include "XBallPlayerState.h"
#include "XBallGameSession.h"

ALobbyGameModeBase::ALobbyGameModeBase()
{
	PlayerControllerClass = AXBallPlayerControllerBase::StaticClass();
	PlayerStateClass = AXBallPlayerState::StaticClass();
	GameSessionClass = AXBallGameSession::StaticClass();
}

void ALobbyGameModeBase::LoadGameMapSeamless(FString MapURL)
{
	//Store Data to GameInstance
	UXBallGameInstanceNative* GameInstance= Cast<UXBallGameInstanceNative>(UGameplayStatics::GetGameInstance(this));
	if (GameInstance)
	{
		GameInstance->StoredFloat.FindOrAdd("bIsTeamPlay") = bIsTeamPlay;
		GameInstance->StoredFloat.FindOrAdd("MaxPlayer") = MaxPlayer;
		GameInstance->StoredFloat.FindOrAdd("TargetScore") = TargetScore;
		GameInstance->StoredFloat.FindOrAdd("InitMoney") = InitMoney;
		GameInstance->StoredFloat.FindOrAdd("MaxE") = MaxE;
		GameInstance->StoredFloat.FindOrAdd("MaxW") = MaxW;
		GameInstance->StoredFloat.FindOrAdd("MaxH") = MaxH;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed To Cast GameInstance!"));
	}
	GetWorld()->ServerTravel(MapURL, true);
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, "Seamless Travel Finished");
}

void ALobbyGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	AXBallPlayerControllerBase* NewXBallPlayerController = Cast<AXBallPlayerControllerBase>(NewPlayer);
	if (!NewXBallPlayerController)
		return;
	if (!bIsTeamPlay)
	{
		NewXBallPlayerController->SetTeam(0);
	}
	if (NewXBallPlayerController->GetXBallPlayerState())
	{
		TArray<AActor*> Controllers;
		TArray<int> TeamNumber;
		TeamNumber.SetNum(5, false);
		UGameplayStatics::GetAllActorsOfClass(this, AXBallPlayerControllerBase::StaticClass(), Controllers);
		for (int i=0;i<Controllers.Num();i++)
		{
			AXBallPlayerControllerBase* tmpController = Cast<AXBallPlayerControllerBase>(Controllers[i]);
			if (tmpController!=NewXBallPlayerController)
				TeamNumber[tmpController->GetTeam()]++;
		}
		int MinTeam=1;
		int ValidTeam=0;
		for (int i=1;i<TeamNumber.Num();i++)
		{
			if (TeamNumber[i]>0)
			{
				ValidTeam++; 
				if (TeamNumber[i] < TeamNumber[MinTeam])
				MinTeam = i;
			}
			
		}
		if (ValidTeam<=1)
		{
			for (int i = 1; i < TeamNumber.Num(); i++)
			{
				if (TeamNumber[i] == 0)
					NewXBallPlayerController->SetTeam(i);
			}
		}
		else
			NewXBallPlayerController->SetTeam(MinTeam);
	}
}

void ALobbyGameModeBase::ClearTeam()
{
	bIsTeamPlay = false;
	TArray<AActor*> Controllers;
	UGameplayStatics::GetAllActorsOfClass(this, AXBallPlayerControllerBase::StaticClass(), Controllers);
	for (int i = 0; i < Controllers.Num(); i++)
	{
		AXBallPlayerControllerBase* tmpController = Cast<AXBallPlayerControllerBase>(Controllers[i]);
		tmpController->SetTeam(0);
	}
}

void ALobbyGameModeBase::RePlaceTeam()
{
	bIsTeamPlay = true;
	TArray<AActor*> Controllers;
	UGameplayStatics::GetAllActorsOfClass(this, AXBallPlayerControllerBase::StaticClass(), Controllers);
	for (int i = 0; i < Controllers.Num()/2; i++)
	{
		AXBallPlayerControllerBase* tmpController = Cast<AXBallPlayerControllerBase>(Controllers[i]);
		tmpController->SetTeam(1);
	}
	for (int i= Controllers.Num() / 2;i<Controllers.Num();i++)
	{
		AXBallPlayerControllerBase* tmpController = Cast<AXBallPlayerControllerBase>(Controllers[i]);
		tmpController->SetTeam(3);
	}
}
