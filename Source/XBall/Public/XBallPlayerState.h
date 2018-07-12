// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "XBallPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class XBALL_API AXBallPlayerState : public APlayerState
{
	GENERATED_BODY()

	TMap<FString, UTexture2D*> CustomTextures;
	TMap<FString, TArray<uint8>> CustomTexturesData;
public:
	UFUNCTION(BlueprintCallable,NetMulticast,Reliable)
		void SetCustomTexture(const FString& TextureParamterName,const TArray<uint8>& TextureData);
	UFUNCTION(BlueprintCallable)
		inline TMap<FString, UTexture2D*> GetCustomTextures()
	{
		return CustomTextures;
	}
};
