// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LobbyGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class XBALL_API ALobbyGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite)
		bool bIsTeamPlay=false;
	UPROPERTY(BlueprintReadWrite)
		int MaxPlayer=16;
	UPROPERTY(BlueprintReadWrite)
		int TargetScore=8;
	UPROPERTY(BlueprintReadWrite)
		int InitMoney=800;
	UFUNCTION(BlueprintCallable)
		void LoadGameMapSeamless(FString MapURL);

	virtual void PostSeamlessTravel() override;

};
