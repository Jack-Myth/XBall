// Fill out your copyright notice in the Description page of Project Settings.

#include "XBallPlayerControllerBase.h"
#include "MyBPFuncLib.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

void AXBallPlayerControllerBase::ReGenOldMap_Implementation(UClass* MapGenerator,int MaxEngth, int MaxWidth, int MaxHeight, int32 Seed,const TArray<FBlockInfo>& BlockInfo)
{
	GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Blue, "Controller Executed RPC");
	if (this->HasAuthority())
	{
		GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, "Warning:Server Is Executeing the ReGenOldMap()!");
	}
	else
		GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Green, "OK:Client Is Executeing the ReGenOldMap().");
	UMyBPFuncLib::GenWorld(this, MaxEngth, MaxWidth, MaxHeight, Seed);
	UMyBPFuncLib::SyncMap(this,BlockInfo);
}

bool AXBallPlayerControllerBase::ReGenOldMap_Validate(UClass* MapGenerator,int MaxEngth, int MaxWidth, int MaxHeight, int32 Seed, const TArray<FBlockInfo>& BlockInfo)
{
	return true;
}

AXBallPlayerControllerBase::AXBallPlayerControllerBase()
{
	ActionClasses.SetNum(7);
}

bool AXBallPlayerControllerBase::SetPlayerDefaultCharacter_Validate(TSubclassOf<AXBallBase> DefaultCharacter)
{
	return true;
}

bool AXBallPlayerControllerBase::SetActionClass_Validate(int Index, TSubclassOf<AActionBase> NewActionClass)
{
	return true;
}

void AXBallPlayerControllerBase::SetPlayerDefaultCharacter_Implementation(TSubclassOf<AXBallBase> DefaultCharacter)
{
	PlayerDefaultCharacter = DefaultCharacter;
}

void AXBallPlayerControllerBase::SetActionClass_Implementation(int Index, TSubclassOf<AActionBase> NewActionClass)
{
	if (Index < 0 || Index>7)
		return;
	ActionClasses[Index] = NewActionClass;
}
