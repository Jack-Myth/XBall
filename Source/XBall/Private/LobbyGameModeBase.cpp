// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyGameModeBase.h"
#include "Engine/World.h"
#include "UserWidget.h"
#include "WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "XBallGameModeBase.h"

void ALobbyGameModeBase::LoadGameMapSeamless(FString MapURL)
{
	GetWorld()->ServerTravel(MapURL, true);
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, "Seamless Travel Finished");
}

void ALobbyGameModeBase::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();
	AXBallGameModeBase* XBallGameMode = Cast<AXBallGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (XBallGameMode)
	{
		XBallGameMode->InitXBallGame(bIsTeamPlay, TargetScore, InitMoney);
	}
}
