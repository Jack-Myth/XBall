// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IMapGenerator.h"
#include "CircleOnlyMapGenerator.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class FIREBALL_API ACircleOnlyMapGenerator:public AActor,public IMapGenerator
{
	GENERATED_BODY()

	bool LocationShouldHaveBlock(FVector BlockXYZ, FVector MaxXYZ);
	bool ShouldPlaceBlock(FVector BlockXYZ, FVector MaxXYZ);
	void SetBlockHealth(FVector BlockLocation,float newHealth);
	void RemoveBlockHealth(FVector BlockLocation);
	float GetBlockHealth(FVector BlockLocation);
	FVector TransformWorldToBlock(FVector WorldLocation);
	FVector TransformWorldToBlockWithNormal(FVector WorldLocation, FVector SurfaceNormal);

	TMap<int, TMap<int, TMap<int, int>>> MapVariation;  //X(Y(Z))
	TMap<int, TMap<int, TMap<int, float>>> MapBlockHealth;
	int* BlockIndexOnLocation(FVector LocationInBlockCoord);
	bool BlockIndexExist(FVector LocationInBlockCoord);
	bool IsLocationHaveBlock_Internal(FVector BlockLocation);
	TArray<FBlockInfo> BreakBlock_Internal(FVector mBlockLocation);
	TArray<FBlockInfo> HitBlock_Internal(FVector mBlockLocation, float Damage);
	FVector MaxXYZ;
public:
	UPROPERTY(BlueprintReadOnly)
		class UInstancedStaticMeshComponent* MapBlockInstance = nullptr;

	UPROPERTY(BlueprintReadOnly)
		AActor* MapHolder=nullptr;

	UPROPERTY(BlueprintReadOnly)
		class UMaterialInstanceDynamic* BlockDMI = nullptr;

	ACircleOnlyMapGenerator();
	~ACircleOnlyMapGenerator();

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObj"))
		virtual void GenMapBlockInstance_Implementation(UObject* WorldContextObj, int MaxEngth, int MaxWidth, int MaxHeight,int Seed=0) override;

	UFUNCTION(BlueprintCallable)
		virtual void BreakBlock_Implementation(FVector mBlockLocation) override;

	UFUNCTION(BlueprintCallable)
		virtual bool IsLocationHaveBlock_Implementation(FVector mBlockLocation)  override;

	UFUNCTION(BlueprintCallable)
		virtual void HitBlock_Implementation(FVector mBlockLocation, float Damage) override;
	UFUNCTION(BlueprintCallable)
		virtual void HitBlockWithNormal_Implementation(FVector mBlockLocation, FVector Normal, float Damage);

	UFUNCTION(BlueprintCallable)
		virtual void BreakBlockRange_Implementation(FVector Center, FVector RangeXYZ, bool ShouldIncludeEdge) override;

	UFUNCTION(BlueprintCallable)
		virtual void HitBlockRange_Implementation(FVector Center, FVector RangeXYZ, float MaxDamage) override;

	UFUNCTION(BlueprintCallable)
		virtual class UMaterialInstanceDynamic* GetDMI_Implementation() override;

	UFUNCTION(BlueprintCallable)
		virtual void SyncMap_Implementation(const TArray<FBlockInfo>& BlockModifiedInfo) override;

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, WithValidation)
		void SyncMap_Rep(const TArray<FBlockInfo>& BlockModifiedInfo);
	UFUNCTION(BlueprintCallable)
		virtual TArray<FBlockInfo> CollectBlockModifiedInfo_Implementation() override;

};
