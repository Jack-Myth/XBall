// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyGameModeBase.h"
#include "Engine/World.h"
#include "UserWidget.h"
#include "WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "XBallGameModeBase.h"
#include "XBallGameInstanceNative.h"

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
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed To Cast GameInstance!"));
	}
	GetWorld()->ServerTravel(MapURL, true);
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, "Seamless Travel Finished");
}
