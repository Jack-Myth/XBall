// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionBase.h"
#include "SkillBase.generated.h"

/**
 * 
 */
UCLASS(BlueprintType,Blueprintable)
class FIREBALL_API ASkillBase : public AActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
		void SkillLeave();
};
