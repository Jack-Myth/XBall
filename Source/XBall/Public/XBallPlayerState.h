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
	UPROPERTY(BlueprintReadWrite,Replicated)
		int KillScore;
	UPROPERTY(BlueprintReadWrite, Replicated)
		int DeadCount;
	UPROPERTY(BlueprintReadOnly, Replicated)
		int Team = 0;

	UFUNCTION(BlueprintCallable,NetMulticast,Reliable)
		void SetCustomTexture(const FString& TextureParamterName,const TArray<uint8>& TextureData);
	UFUNCTION(BlueprintPure)
		inline TMap<FString, UTexture2D*> GetCustomTextures()
	{
		return CustomTextures;
	}

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const override;

};
