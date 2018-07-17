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

	UPROPERTY(Replicated)
		float CoolDown=0;
	UPROPERTY()
		float MaxCoolDown = 3;
	FTimerHandle CoolDownTimeHandle;
public:
	UFUNCTION(BlueprintCallable)
		void SkillLeave();
	
	UFUNCTION(BlueprintCallable,Server,Reliable,WithValidation)
		void UpdateCoolDown(float CoolDownTime);

	UFUNCTION(BlueprintNativeEvent)
		void SelectedWhileCD();

	UFUNCTION(BlueprintPure)
		bool IsCoolingDown();

	UFUNCTION(BlueprintPure)
		float GetCoolDownRemain();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
		virtual float GetProgressValue_Implementation() override;

};
