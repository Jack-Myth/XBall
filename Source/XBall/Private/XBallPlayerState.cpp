// Fill out your copyright notice in the Description page of Project Settings.

#include "XBallPlayerState.h"
#include "MyBPFuncLib.h"
#include "UnrealNetwork.h"

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

void AXBallPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
	DOREPLIFETIME(AXBallPlayerState, KillScore);
	DOREPLIFETIME(AXBallPlayerState, DeadCount);
	DOREPLIFETIME(AXBallPlayerState, Team);
}
