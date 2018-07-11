// Fill out your copyright notice in the Description page of Project Settings.

#include "MyBPFuncLib.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Object.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialParameterCollection.h"

/*void UMyBPFuncLib::SetMapGenerator(UMapGenerator* newMapGenerator)
{
	CoreMapGenerator = newMapGenerator;
}*/

void UMyBPFuncLib::GenWorld(UObject* WorldContextObj, int Ength, int Width, int Height, int32 Seed)
{
	TArray<AActor*> CoreMapGenerators;
	UGameplayStatics::GetAllActorsWithInterface(WorldContextObj, UMapGenerator::StaticClass(), CoreMapGenerators);
	if (!CoreMapGenerators.Num())
		return;
	IMapGenerator::Execute_GenMapBlockInstance(CoreMapGenerators[0], WorldContextObj, Ength, Width, Height,Seed);
	FTimerHandle InitAnimationTimeHandle;
	TArray<UMaterialInstanceDynamic*> BlockDMIs = IMapGenerator::Execute_GetDMIs(CoreMapGenerators[0]);
	UMaterialParameterCollection* MPC = LoadObject<UMaterialParameterCollection>(nullptr, TEXT("MaterialParameterCollection'/Game/FireBall/Materials/MatData.MatData'"));
	UKismetMaterialLibrary::SetScalarParameterValue(WorldContextObj, MPC, "CubeSize", 50);
	UKismetMaterialLibrary::SetVectorParameterValue(WorldContextObj, MPC, "ScaleOrigin", FLinearColor(FVector(0,0,0)));
	WorldContextObj->GetWorld()->GetTimerManager().SetTimer(InitAnimationTimeHandle, [=]()
		{
			int x = BlockDMIs[0]->K2_GetScalarParameterValue("ScaleRange") + 30;
			for (int i = 0; i < BlockDMIs.Num(); i++)
			{
				BlockDMIs[i]->SetScalarParameterValue("ScaleRange", x);
			}
			if (x > 655360)
			{
				FTimerHandle tmpTimerHandle = InitAnimationTimeHandle;
				WorldContextObj->GetWorld()->GetTimerManager().ClearTimer(tmpTimerHandle);
			}
		}, 0.02f, true, -1);
}

void UMyBPFuncLib::HitBlockAtLocation(UObject* WorldContextObj, FVector HitWorldLocation, float Damage)
{
	TArray<AActor*> CoreMapGenerators;
	UGameplayStatics::GetAllActorsWithInterface(WorldContextObj, UMapGenerator::StaticClass(), CoreMapGenerators);
	if (!CoreMapGenerators.Num())
		return;
	IMapGenerator::Execute_HitBlock(CoreMapGenerators[0], HitWorldLocation, Damage);
}

void UMyBPFuncLib::HitBlockAtLocationWithNormal(UObject* WorldContextObj, FVector HitWorldLocation, FVector SurfaceNormal, float Damage)
{
	TArray<AActor*> CoreMapGenerators;
	UGameplayStatics::GetAllActorsWithInterface(WorldContextObj, UMapGenerator::StaticClass(), CoreMapGenerators);
	if (!CoreMapGenerators.Num())
		return;
	IMapGenerator::Execute_HitBlockWithNormal(CoreMapGenerators[0], HitWorldLocation,SurfaceNormal, Damage);
}

void UMyBPFuncLib::BreakBlockAtLocation(UObject* WorldContextObj, FVector WorldLocation)
{
	TArray<AActor*> CoreMapGenerators;
	UGameplayStatics::GetAllActorsWithInterface(WorldContextObj, UMapGenerator::StaticClass(), CoreMapGenerators);
	if (!CoreMapGenerators.Num())
		return;
	IMapGenerator::Execute_BreakBlock(CoreMapGenerators[0], WorldLocation);
}

void UMyBPFuncLib::BreakBlockAtRange(UObject* WorldContextObj, FVector WorldLocation, FVector RangeXYZ, bool ShouldIncludeEdge)
{
	TArray<AActor*> CoreMapGenerators;
	UGameplayStatics::GetAllActorsWithInterface(WorldContextObj, UMapGenerator::StaticClass(), CoreMapGenerators);
	if (!CoreMapGenerators.Num())
		return;
	IMapGenerator::Execute_BreakBlockRange(CoreMapGenerators[0], WorldLocation, RangeXYZ, ShouldIncludeEdge);
}

void UMyBPFuncLib::HitBlockAtRange(UObject* WorldContextObj, FVector WorldLocation, FVector RangeXYZ, float MaxDamage)
{
	TArray<AActor*> CoreMapGenerators;
	UGameplayStatics::GetAllActorsWithInterface(WorldContextObj, UMapGenerator::StaticClass(), CoreMapGenerators);
	if (!CoreMapGenerators.Num())
		return;
	IMapGenerator::Execute_HitBlockRange(CoreMapGenerators[0], WorldLocation, RangeXYZ, MaxDamage);
}

void UMyBPFuncLib::SyncMap(UObject* WorldContextObj, const TArray<FBlockInfo>& BlockModifiedInfo)
{
	TArray<AActor*> CoreMapGenerators;
	UGameplayStatics::GetAllActorsWithInterface(WorldContextObj, UMapGenerator::StaticClass(), CoreMapGenerators);
	if (!CoreMapGenerators.Num())
		return;
	IMapGenerator::Execute_SyncMap(CoreMapGenerators[0],BlockModifiedInfo);
}

TArray<FBlockInfo> UMyBPFuncLib::CollectMapModified(UObject* WorldContextObj)
{
	TArray<AActor*> CoreMapGenerators;
	UGameplayStatics::GetAllActorsWithInterface(WorldContextObj, UMapGenerator::StaticClass(), CoreMapGenerators);
	if (!CoreMapGenerators.Num())
		return TArray<FBlockInfo>();
	return IMapGenerator::Execute_CollectBlockModifiedInfo(CoreMapGenerators[0]);
}