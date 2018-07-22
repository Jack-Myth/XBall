// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "XBallGameInstanceNative.generated.h"

/**
 * 
 */
UCLASS()
class XBALL_API UXBallGameInstanceNative : public UGameInstance
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite)
		FString ServerName;
	UPROPERTY(BlueprintReadWrite)
		TMap<FName, FString> StoredString;
	UPROPERTY(BlueprintReadWrite)
		TMap<FName, float> StoredFloat;
};
