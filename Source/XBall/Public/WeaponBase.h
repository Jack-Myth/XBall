// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionBase.h"
#include "WeaponBase.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class XBALL_API AWeaponBase : public AActionBase
{
	GENERATED_BODY()

public: 

	UFUNCTION(BlueprintImplementableEvent)
		void BeginFire(FVector TargetLocation);

	UFUNCTION(BlueprintImplementableEvent)
		void EndFire(FVector TargetLocation);

	UFUNCTION(BlueprintImplementableEvent)
		void OnSwitched();
};
