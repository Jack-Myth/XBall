// Fill out your copyright notice in the Description page of Project Settings.

#include "XBallPlayerState.h"
#include "MyBPFuncLib.h"
#include "UnrealNetwork.h"
#include "XBallBase.h"
#include "Kismet/GameplayStatics.h"

void AXBallPlayerState::SetCustomTexture_Implementation(const FString& TextureParamterName, const TArray<uint8>& TextureData)
{
	if (TextureData.Num())
	{
		CustomTexturesData.FindOrAdd(TextureParamterName) = TextureData;
		CustomTextures.FindOrAdd(TextureParamterName) = UMyBPFuncLib::GetTextureFromData(TextureData);
	}
	else
	{
		CustomTexturesData.Remove(TextureParamterName);
		CustomTextures.Remove(TextureParamterName);
	}
}

void AXBallPlayerState::SetLobbyIsReady_Implementation(bool Ready)
{
	Lobby_IsReady = Ready;
}

bool AXBallPlayerState::SetLobbyIsReady_Validate(bool Ready)
{
	return true;
}

void AXBallPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AXBallPlayerState, KillScore);
	DOREPLIFETIME(AXBallPlayerState, DeadCount);
	DOREPLIFETIME(AXBallPlayerState, Team);
	DOREPLIFETIME(AXBallPlayerState, Lobby_IsReady);
}

void AXBallPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);
	AXBallPlayerState* XBallPlayerState = Cast<AXBallPlayerState>(PlayerState);
	if (XBallPlayerState)
	{
		//KillScore And DeadCount may use for out-game rank
		XBallPlayerState->KillScore = KillScore;
		XBallPlayerState->DeadCount = DeadCount;

		XBallPlayerState->Team = Team;
	}
}
