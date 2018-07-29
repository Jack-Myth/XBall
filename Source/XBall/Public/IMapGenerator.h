// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IMapGenerator.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UMapGenerator : public UInterface
{
	GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct FBlockInfo
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
		int X;
	UPROPERTY(BlueprintReadWrite)
		int Y;
	UPROPERTY(BlueprintReadWrite)
		int Z;
	UPROPERTY(BlueprintReadWrite)
		int Value;
};

/**
 * 
 */
class XBALL_API IMapGenerator
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent,meta=(WorldContext="WorldContextObj"))
		void GenMapBlockInstance(UObject* WorldContextObj,int MaxEngth,int MaxWidth,int MaxHeight, int32 Seed=0);
	
	UFUNCTION(BlueprintNativeEvent)
		void BreakBlock(FVector BlockLocation);

	UFUNCTION(BlueprintNativeEvent)
		bool IsLocationHaveBlock(FVector BlockLocation);

	UFUNCTION(BlueprintNativeEvent)
		void HitBlock(FVector BlockLocation,float Damage);

	UFUNCTION(BlueprintNativeEvent)
		void HitBlockWithNormal(FVector BlockLocation,FVector Normal, float Damage);

	UFUNCTION(BlueprintNativeEvent)
		void BreakBlockRange(FVector Center, FVector RangeXYZ,bool ShouldIncludeEdge);

	UFUNCTION(BlueprintNativeEvent)
		void HitBlockRange(FVector Center, FVector RangeXYZ,float MaxDamage);

	UFUNCTION(BlueprintNativeEvent)
		TArray<class UMaterialInstanceDynamic*> GetDMIs();

	UFUNCTION(BlueprintNativeEvent)
		void SyncMap(const TArray<FBlockInfo>& BlockModifiedInfo);
	/*UFUNCTION(BlueprintImplementableEvent)
		void RegisterUsefulBlock()*/

	UFUNCTION(BlueprintNativeEvent)
		TArray<FBlockInfo> CollectBlockModifiedInfo();

	UFUNCTION(BlueprintNativeEvent)
		void ClearMap();
};
