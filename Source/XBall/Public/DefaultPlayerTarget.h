// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DecalActor.h"
#include "ScenePlayerTarget.h"
#include "DefaultPlayerTarget.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class XBALL_API ADefaultPlayerTarget : public ADecalActor,public IScenePlayerTarget
{
	GENERATED_BODY()
public:
	ADefaultPlayerTarget();
	UFUNCTION(BlueprintCallable)
	virtual void OnUpdateTargetPosition_Implementation(FVector WorldLocation, FVector SurfaceNormal) override;
	UFUNCTION(BlueprintCallable)
	virtual void NotifyDestruction_Implementation() override;
};
