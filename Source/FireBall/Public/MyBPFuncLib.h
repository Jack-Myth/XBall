// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "IMapGenerator.h"
#include "MyBPFuncLib.generated.h"

/**
 * 
 */
UCLASS()
class FIREBALL_API UMyBPFuncLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta=(WorldContext="WorldContextObj"))
		static void GenWorld(UObject* WorldContextObj, int Ength, int Width, int Height, int32 Seed);
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObj"))
		static void HitBlockAtLocation(UObject* WorldContextObj, FVector HitWorldLocation, float Damage);
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObj"))
		static void HitBlockAtLocationWithNormal(UObject* WorldContextObj, FVector HitWorldLocation,FVector SurfaceNormal, float Damage);
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObj"))
		static void BreakBlockAtLocation(UObject* WorldContextObj, FVector WorldLocation);
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObj"))
		static void BreakBlockAtRange(UObject* WorldContextObj, FVector WorldLocation,FVector RangeXYZ,bool ShouldIncludeEdge);
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObj"))
		static void  HitBlockAtRange(UObject* WorldContextObj, FVector WorldLocation,FVector RangeXYZ, float MaxDamage);
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObj"))
		static void SyncMap(UObject* WorldContextObj, const TArray<FBlockInfo>& BlockModifiedInfo);
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObj"))
		static TArray<FBlockInfo> CollectMapModified(UObject* WorldContextObj);
};
