// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionBase.h"
#include "SkillBase.generated.h"

/**
 * 
 */
UCLASS(BlueprintType,Blueprintable)
class XBALL_API ASkillBase : public AActionBase
{
	GENERATED_BODY()

	float CoolDown=0;
public:
	UFUNCTION(BlueprintCallable)
		void SkillLeave();
	
	UFUNCTION(BlueprintCallable)
		void UpdateCoolDown(float CoolDownTime);

	UFUNCTION(BlueprintNativeEvent)
		void SelectedWhileCD();

	UFUNCTION(BlueprintCallable)
		bool IsCoolingDown();
};
