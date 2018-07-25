// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameSession.h"
#include "XBallGameSession.generated.h"

/**
 * 
 */
UCLASS()
class XBALL_API AXBallGameSession : public AGameSession
{
	GENERATED_BODY()

	virtual void PostLogin(APlayerController* NewPlayer) override;

};
