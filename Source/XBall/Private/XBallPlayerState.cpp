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
		if (TextureParamterName=="BaseColor")
		{
			BaseTextureData = TextureData;
		}
		else if (TextureParamterName=="NormalMap")
		{
			NormalMapData = TextureData;
		}
		CustomTextures.FindOrAdd(TextureParamterName) = UMyBPFuncLib::GetTextureFromData(TextureData);
	}
	else
	{
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

bool AXBallPlayerState::SetCustomTexture_Validate(const FString& TextureParamterName, const TArray<uint8>& TextureData)
{
	if (TextureData.Num()>256*1024)
	{
		return false;
	}
	return true;
}

void AXBallPlayerState::BaseTextureData_Rep()
{
	if (BaseTextureData.Num())
		CustomTextures.FindOrAdd("BaseTexture") = UMyBPFuncLib::GetTextureFromData(BaseTextureData);
	else
		CustomTextures.FindOrAdd("BaseTexture") = nullptr;
}

void AXBallPlayerState::NormalMapData_Rep()
{
	if (NormalMapData.Num())
		CustomTextures.FindOrAdd("NormalMap") = UMyBPFuncLib::GetTextureFromData(NormalMapData);
	else
		CustomTextures.FindOrAdd("NormalMap") = nullptr;
}

void AXBallPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AXBallPlayerState, KillScore);
	DOREPLIFETIME(AXBallPlayerState, DeadCount);
	DOREPLIFETIME(AXBallPlayerState, Team);
	DOREPLIFETIME(AXBallPlayerState, Lobby_IsReady);
	DOREPLIFETIME(AXBallPlayerState, NormalMapData);
	DOREPLIFETIME(AXBallPlayerState, BaseTextureData);
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
