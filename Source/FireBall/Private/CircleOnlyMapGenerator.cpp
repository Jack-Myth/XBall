// Fill out your copyright notice in the Description page of Project Settings.

#include "CircleOnlyMapGenerator.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "SimplexNoiseBPLibrary.h"
#include "time.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/SceneComponent.h"

bool ACircleOnlyMapGenerator::LocationShouldHaveBlock(FVector BlockXYZ, FVector MaxXYZ)
{
	return abs(BlockXYZ.Z) > MaxXYZ.Z *0.4 ||
		abs(BlockXYZ.Y) > MaxXYZ.Y *0.4 ||
		abs(BlockXYZ.X) > MaxXYZ.X *0.4 || 
		USimplexNoiseBPLibrary::SimplexNoise3D(BlockXYZ.X / 20.f, BlockXYZ.Y / 20.f, BlockXYZ.Z / 10.f) > 0.5;
}

bool ACircleOnlyMapGenerator::ShouldPlaceBlock(FVector BlockXYZ, FVector MaxXYZ)
{
	if (!LocationShouldHaveBlock(BlockXYZ, MaxXYZ))
		return false;
	if (!LocationShouldHaveBlock(BlockXYZ + FVector(1, 0, 0), MaxXYZ) ||
		!LocationShouldHaveBlock(BlockXYZ + FVector(-1, 0, 0), MaxXYZ) ||
		!LocationShouldHaveBlock(BlockXYZ + FVector(0, 1, 0), MaxXYZ) ||
		!LocationShouldHaveBlock(BlockXYZ + FVector(0, -1, 0), MaxXYZ) ||
		!LocationShouldHaveBlock(BlockXYZ + FVector(0, 0, 1), MaxXYZ) ||
		!LocationShouldHaveBlock(BlockXYZ + FVector(0, 0, -1), MaxXYZ))
		return true;
	return false;
}

void ACircleOnlyMapGenerator::SetBlockHealth(FVector BlockLocation, float newHealth)
{
	MapBlockHealth.FindOrAdd(BlockLocation.X).FindOrAdd(BlockLocation.Y).FindOrAdd(BlockLocation.Z) = newHealth;
}

void ACircleOnlyMapGenerator::RemoveBlockHealth(FVector BlockLocation)
{
	TMap<int, TMap<int, float>>* tmpX;
	TMap<int, float>	*tmpY;
	float* tmpZ;
	tmpX = MapBlockHealth.Find(BlockLocation.X);
	if (tmpX)
		tmpY = tmpX->Find(BlockLocation.Y);
	else
		return;
	if (tmpY)
		tmpZ = tmpY->Find(BlockLocation.Z);
	else
		return;
	if (tmpZ)
	{
		tmpY->Remove(BlockLocation.Z);
		if (tmpY->Num() == 0)
		{
			tmpX->Remove(BlockLocation.Y);
			if (tmpX->Num() == 0)
				MapBlockHealth.Remove(BlockLocation.X);
		}
	}

}

float ACircleOnlyMapGenerator::GetBlockHealth(FVector BlockLocation)
{
	TMap<int, TMap<int, float>>* tmpX;
	TMap<int, float>	*tmpY;
	float* tmpZ;
	tmpX = MapBlockHealth.Find(BlockLocation.X);
	if (tmpX)
		tmpY = tmpX->Find(BlockLocation.Y);
	else
		return -1;
	if (tmpY)
		tmpZ = tmpY->Find(BlockLocation.Z);
	else
		return -1;
	if (tmpZ)
		return *tmpZ;
	return -1;
}

FVector ACircleOnlyMapGenerator::TransformWorldToBlock(FVector WorldLocation)
{
	WorldLocation.X = (int)((abs(WorldLocation.X) / 100.f + 0.5f)*(WorldLocation.X >= 0 ? 1 : -1));
	WorldLocation.Y = (int)((abs(WorldLocation.Y) / 100.f + 0.5f)*(WorldLocation.Y >= 0 ? 1 : -1));
	float ZNat = (WorldLocation.Z >= 0 ? 1 : -1);
	WorldLocation.Z = (int)((abs(WorldLocation.Z) / 100.f + (ZNat > 0 ? 0 : 1.f))*ZNat);
	return WorldLocation;
}

FVector ACircleOnlyMapGenerator::TransformWorldToBlockWithNormal(FVector WorldLocation, FVector SurfaceNormal)
{
	FVector tmpLocation = TransformWorldToBlock(WorldLocation);
	if (FVector::DotProduct(SurfaceNormal, FVector(1, 0, 0))>0.8f)
	{
		tmpLocation.X= FMath::FloorToFloat(WorldLocation.X/100.f);
		return tmpLocation;
	}
	if (FVector::DotProduct(SurfaceNormal, FVector(-1, 0, 0)) > 0.8f)
	{
		tmpLocation.X = FMath::CeilToFloat(WorldLocation.X/100.f);
		return tmpLocation;
	}
	if (FVector::DotProduct(SurfaceNormal, FVector(0, 1, 0)) > 0.8f)
	{
		tmpLocation.Y = FMath::FloorToFloat(WorldLocation.Y/100.f);
		return tmpLocation;
	}
	if (FVector::DotProduct(SurfaceNormal, FVector(0, -1, 0)) > 0.8f)
	{
		tmpLocation.Y = FMath::CeilToFloat(WorldLocation.Y/100.f);
		return tmpLocation;
	}
	if (FVector::DotProduct(SurfaceNormal, FVector(0, 0, 1)) > 0.8f)
	{
		WorldLocation.Z -= 50;
		tmpLocation.Z = FMath::FloorToFloat(WorldLocation.Z/100.f);
		return tmpLocation;
	}
	if (FVector::DotProduct(SurfaceNormal, FVector(0, 0, -1)) > 0.8f)
	{
		WorldLocation.Z -= 50;
		tmpLocation.Z = FMath::CeilToFloat(WorldLocation.Z / 100.f);
		return tmpLocation;
	}
	return tmpLocation;
}

int* ACircleOnlyMapGenerator::BlockIndexOnLocation(FVector LocationInBlockCoord)
{
	void* tmpP;
	tmpP = MapVariation.Find(LocationInBlockCoord.X);
	if (tmpP)
		tmpP = ((TMap<int, TMap<int, int>>*)tmpP)->Find(LocationInBlockCoord.Y);
	if (tmpP)
		tmpP = ((TMap<int, int>*)tmpP)->Find(LocationInBlockCoord.Z);
	if (tmpP&&*(int*)tmpP >= 0)
		return (int*)tmpP;
	return nullptr;
}

bool ACircleOnlyMapGenerator::BlockIndexExist(FVector LocationInBlockCoord)
{
	void* tmpP;
	tmpP = MapVariation.Find(LocationInBlockCoord.X);
	if (tmpP)
		tmpP = ((TMap<int, TMap<int, int>>*)tmpP)->Find(LocationInBlockCoord.Y);
	if (tmpP)
		tmpP = ((TMap<int, int>*)tmpP)->Find(LocationInBlockCoord.Z);
	if (tmpP)
		return true;
	return false;
}

bool ACircleOnlyMapGenerator::IsLocationHaveBlock_Internal(FVector BlockLocation)
{
	if (!BlockIndexExist(BlockLocation))
		return LocationShouldHaveBlock(BlockLocation, MaxXYZ);
	else
		return BlockIndexOnLocation(BlockLocation);
}

TArray<FBlockInfo> ACircleOnlyMapGenerator::HitBlock_Internal(FVector mBlockLocation, float Damage)
{
	int* tmpP = BlockIndexOnLocation(mBlockLocation);
	FBlockInfo curBlockInfo;
	curBlockInfo.X = mBlockLocation.X;
	curBlockInfo.Y = mBlockLocation.Y;
	curBlockInfo.Z = mBlockLocation.Z;
	if (IsLocationHaveBlock_Internal(mBlockLocation))
	{
		float tmpHealth = GetBlockHealth(mBlockLocation);
		if (tmpHealth == -1)
		{
			// For different Block type, Give Different MaxHealth
			int MaxHealth=100;
			if (abs(mBlockLocation.X) > MaxXYZ.X - 1 ||
				abs(mBlockLocation.Y) > MaxXYZ.Y - 1 ||
				abs(mBlockLocation.Z) > MaxXYZ.Z - 1)
			{
				MaxHealth = 700;
			}
			else
			{
				switch ((int)(USimplexNoiseBPLibrary::SimplexNoise3D(mBlockLocation.X, mBlockLocation.Y, mBlockLocation.Z) * 10))
				{
					case 5:
						MaxHealth = 100;
						break;
					case 6:
						MaxHealth = 200;
						break;
					case 7:
						MaxHealth = 300;
						break;
					case 8:
						MaxHealth = 400;
						break;
					case 9:
						MaxHealth = 500;
						break;
				}
			}
			if (Damage >= MaxHealth)
			{
				curBlockInfo.Value = -1;
				BreakBlock_Internal(mBlockLocation);
			}
			else
			{
				SetBlockHealth(mBlockLocation, MaxHealth - Damage);
				curBlockInfo.Value = MaxHealth - Damage;
			}
		}
		else
		{
			tmpHealth -= Damage;
			if (tmpHealth <= 0)
			{
				BreakBlock_Internal(mBlockLocation);
				curBlockInfo.Value = -1;
				//Spawn Particles;
			}
			else
			{
				SetBlockHealth(mBlockLocation, tmpHealth);
				curBlockInfo.Value = tmpHealth;
			}
		}
		return TArray<FBlockInfo>({ curBlockInfo });
	}
	return TArray<FBlockInfo>();
}

void ACircleOnlyMapGenerator::HitBlockRange_Implementation(FVector Center, FVector RangeXYZ, float MaxDamage)
{
	if (RangeXYZ.X < 0 || RangeXYZ.Y < 0 || RangeXYZ.Z < 0)
		return;
	TArray<FBlockInfo> SyncData;
	FVector BlockMin = TransformWorldToBlock(Center - RangeXYZ);
	FVector BlockMax = TransformWorldToBlock(Center + RangeXYZ);
	for (int z = BlockMin.Z; z <= BlockMax.Z; z++)
		for (int y = BlockMin.Y; y <= BlockMax.Y; y++)
			for (int x = BlockMin.X; x <= BlockMax.X; x++)
			{
				//Caculate Damage Ratio
				FVector BlockPosition = FVector(x, y, z);
				FVector DirVect = Center - BlockPosition * 100;
				float DamageRatio = 1;
				if (DirVect.Size() > 100)
				{
					//Ratio Decreased with Distance
					DirVect.Normalize();
					DirVect *= 50;
					DamageRatio = FMath::Clamp(1 - ((BlockPosition * 100 + DirVect - Center) / RangeXYZ).Size(), 0.f, 1.f);
				}
				else
					DamageRatio = 1;
				SyncData.Append(HitBlock_Internal(BlockPosition, DamageRatio*MaxDamage));
			}
	if (SyncData.Num())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, "Rep has beed send.");
		SyncMap_Rep(SyncData);
	}
}

TArray<FBlockInfo> ACircleOnlyMapGenerator::BreakBlock_Internal(FVector mBlockLocation)
{
	int* tmpP = BlockIndexOnLocation(mBlockLocation);
	if (tmpP&&*tmpP >= 0)
	{
		//MapBlockInstance->RemoveInstance(*(int*)tmpP);
		FTransform BlockOriginTransform;
		MapBlockInstance->GetInstanceTransform(*tmpP, BlockOriginTransform, true);
		BlockOriginTransform.SetScale3D(FVector(0.0f));
		MapBlockInstance->UpdateInstanceTransform(*tmpP, BlockOriginTransform, true, true);
		RemoveBlockHealth(mBlockLocation);
		*tmpP = -1;
		if (!BlockIndexExist(mBlockLocation + FVector(1, 0, 0)) && LocationShouldHaveBlock(mBlockLocation + FVector(1, 0, 0), MaxXYZ))
			MapVariation.FindOrAdd(mBlockLocation.X + 1).FindOrAdd(mBlockLocation.Y).FindOrAdd(mBlockLocation.Z) =
			MapBlockInstance->AddInstanceWorldSpace(FTransform(FRotator(0, 0, 0), FVector(mBlockLocation.X + 1, mBlockLocation.Y, mBlockLocation.Z) * 100, FVector(1.f, 1.f, 1.f)));
		if (!BlockIndexExist(mBlockLocation + FVector(-1, 0, 0)) && LocationShouldHaveBlock(mBlockLocation + FVector(-1, 0, 0), MaxXYZ))
			MapVariation.FindOrAdd(mBlockLocation.X - 1).FindOrAdd(mBlockLocation.Y).FindOrAdd(mBlockLocation.Z) =
			MapBlockInstance->AddInstanceWorldSpace(FTransform(FRotator(0, 0, 0), FVector(mBlockLocation.X - 1, mBlockLocation.Y, mBlockLocation.Z) * 100, FVector(1.f, 1.f, 1.f)));
		if (!BlockIndexExist(mBlockLocation + FVector(0, 1, 0)) && LocationShouldHaveBlock(mBlockLocation + FVector(0, 1, 0), MaxXYZ))
			MapVariation.FindOrAdd(mBlockLocation.X).FindOrAdd(mBlockLocation.Y + 1).FindOrAdd(mBlockLocation.Z) =
			MapBlockInstance->AddInstanceWorldSpace(FTransform(FRotator(0, 0, 0), FVector(mBlockLocation.X, mBlockLocation.Y + 1, mBlockLocation.Z) * 100, FVector(1.f, 1.f, 1.f)));
		if (!BlockIndexExist(mBlockLocation + FVector(0, -1, 0)) && LocationShouldHaveBlock(mBlockLocation + FVector(0, -1, 0), MaxXYZ))
			MapVariation.FindOrAdd(mBlockLocation.X).FindOrAdd(mBlockLocation.Y - 1).FindOrAdd(mBlockLocation.Z) =
			MapBlockInstance->AddInstanceWorldSpace(FTransform(FRotator(0, 0, 0), FVector(mBlockLocation.X, mBlockLocation.Y - 1, mBlockLocation.Z) * 100, FVector(1.f, 1.f, 1.f)));
		if (!BlockIndexExist(mBlockLocation + FVector(0, 0, 1)) && LocationShouldHaveBlock(mBlockLocation + FVector(0, 0, 1), MaxXYZ))
			MapVariation.FindOrAdd(mBlockLocation.X).FindOrAdd(mBlockLocation.Y).FindOrAdd(mBlockLocation.Z + 1) =
			MapBlockInstance->AddInstanceWorldSpace(FTransform(FRotator(0, 0, 0), FVector(mBlockLocation.X, mBlockLocation.Y, mBlockLocation.Z + 1) * 100, FVector(1.f, 1.f, 1.f)));
		if (!BlockIndexExist(mBlockLocation + FVector(0, 0, -1)) && LocationShouldHaveBlock(mBlockLocation + FVector(0, 0, -1), MaxXYZ))
			MapVariation.FindOrAdd(mBlockLocation.X).FindOrAdd(mBlockLocation.Y).FindOrAdd(mBlockLocation.Z - 1) =
			MapBlockInstance->AddInstanceWorldSpace(FTransform(FRotator(0, 0, 0), FVector(mBlockLocation.X, mBlockLocation.Y, mBlockLocation.Z - 1) * 100, FVector(1.f, 1.f, 1.f)));
		return TArray<FBlockInfo>({ FBlockInfo({ (int)mBlockLocation.X,(int)mBlockLocation.Y,(int)mBlockLocation.Z,-1 }) });
	}
	return TArray<FBlockInfo>();
}


ACircleOnlyMapGenerator::ACircleOnlyMapGenerator()
{
	RootComponent= CreateDefaultSubobject<USceneComponent>("RootComponent");
	SetReplicates(true);
}

ACircleOnlyMapGenerator::~ACircleOnlyMapGenerator()
{
}

void ACircleOnlyMapGenerator::GenMapBlockInstance_Implementation(UObject* WorldContextObj, int MaxEngth, int MaxWidth, int MaxHeight,int32 Seed)
{
	USimplexNoiseBPLibrary::setNoiseSeed(Seed);
	MapHolder = WorldContextObj->GetWorld()->SpawnActor<AActor>(FVector(0, 0, 0), FRotator(0, 0, 0));
	MapBlockInstance = NewObject<UInstancedStaticMeshComponent>();
	MapBlockInstance->RegisterComponentWithWorld(WorldContextObj->GetWorld());
	MapBlockInstance->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("StaticMesh'/Game/FireBall/Meshs/SimpleBox.SimpleBox'")));
	UMaterial* tmpMat = LoadObject<UMaterial>(nullptr, TEXT("Material'/Game/FireBall/Materials/BlockBaseMat.BlockBaseMat'"));
	MaxXYZ = FVector(MaxEngth, MaxWidth, MaxHeight);
	//MapBlockInstance->SetMaterial(0, tmpMat);
	BlockDMI=UMaterialInstanceDynamic::Create(tmpMat,MapBlockInstance);
	MapBlockInstance->SetMaterial(0,BlockDMI);
	for (int z=-MaxHeight /2;z<MaxHeight /2;z++)
		for (int y = -MaxWidth / 2; y<MaxWidth / 2; y++)
			for (int x = -MaxEngth / 2; x < MaxEngth/ 2; x++)
			{
				if (ShouldPlaceBlock(FVector(x,y,z), FVector(MaxEngth,MaxWidth,MaxHeight)))
				{
					MapVariation.FindOrAdd(x).FindOrAdd(y).FindOrAdd(z)=
					MapBlockInstance->AddInstanceWorldSpace(FTransform(FRotator(0, 0, 0), FVector(x, y, z) * 100, FVector(1.f, 1.f, 1.f)));
				}
			}
	MapBlockInstance->SetCullDistances(10, 20);
	MapBlockInstance->Rename(TEXT("MapBlockHolder"), MapHolder);
	MapHolder->SetRootComponent(MapBlockInstance);
	if (!MapHolder->GetRootComponent())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, "Error:NULL Root Component From MapHolder!");
		return;
	}
	// InstacedStaticMeshComponent Doesn't Support Replication
	// Disable Replicate otherwise many warning will generate.
	MapHolder->SetReplicates(false);
	MapHolder->bNetLoadOnClient = false;
	
	// Test Only
	/*UStaticMeshComponent* StaticMeshC = NewObject<UStaticMeshComponent>();
	StaticMeshC->RegisterComponentWithWorld(WorldContextObj->GetWorld());
	StaticMeshC->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("StaticMesh'/Game/FireBall/Meshs/SimpleBox.SimpleBox'")));
	StaticMeshC->SetWorldScale3D(FVector(5, 5, 0.5));
	StaticMeshC->SetWorldLocation(FVector(0, 0, -300));*/
}


class UMaterialInstanceDynamic* ACircleOnlyMapGenerator::GetDMI_Implementation()
{
	return BlockDMI;
}

void ACircleOnlyMapGenerator::SyncMap_Implementation(const TArray<FBlockInfo>& BlockModifiedInfo)
{
	for (const FBlockInfo& BlockInfo : BlockModifiedInfo)
	{
		if (BlockInfo.Value <= 0)
			BreakBlock_Internal(FVector(BlockInfo.X, BlockInfo.Y, BlockInfo.Z));
		else
		{
			SetBlockHealth(FVector(BlockInfo.X, BlockInfo.Y, BlockInfo.Z), BlockInfo.Value);
		}
	}
}

void ACircleOnlyMapGenerator::SyncMap_Rep_Implementation(const TArray<FBlockInfo>& BlockModifiedInfo)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, "Rep has beed received.");
	if (this->HasAuthority())
	{
		UE_LOG(LogTemp, Display, TEXT("SyncMap_Rep Is Executing On Server!"));
		//GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, TEXT("SyncMap_Rep Is Executing On Server!"));
		return;
	}
	else
		UE_LOG(LogTemp, Display, TEXT("SyncMap_Rep Is Executing On Client."));
		//GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Green, "SyncMap_Rep Is Executing On Client.");
	IMapGenerator::Execute_SyncMap(this, BlockModifiedInfo);
}

bool ACircleOnlyMapGenerator::SyncMap_Rep_Validate(const TArray<FBlockInfo>& BlockModifiedInfo)
{
	if (!BlockModifiedInfo.Num())
		return false;
	return true;
}

TArray<FBlockInfo> ACircleOnlyMapGenerator::CollectBlockModifiedInfo_Implementation()
{
	TArray<FBlockInfo> Blockinfo;
	for (auto itx=MapVariation.CreateIterator();itx;++itx)
		for (auto ity = itx->Value.CreateIterator(); ity; ++ity)
			for (auto itz = ity->Value.CreateIterator(); itz; ++itz)
			{
				if (itz->Value<0)
				{
					FBlockInfo tmpB= { itx->Key,ity->Key,itz->Key,0 };
					Blockinfo.Add(tmpB);
				}
			}
	for (auto itx = MapBlockHealth.CreateIterator(); itx; ++itx)
		for (auto ity = itx->Value.CreateIterator(); ity; ++ity)
			for (auto itz = ity->Value.CreateIterator(); itz; ++itz)
			{
				if (itz->Value != 100)
				{
					FBlockInfo tmpB = { itx->Key,ity->Key,itz->Key,itz->Value };
					Blockinfo.Add(tmpB);
				}
			}
	return Blockinfo;
}

void ACircleOnlyMapGenerator::BreakBlockRange_Implementation(FVector Center, FVector RangeXYZ, bool ShouldIncludeEdge)
{
	if (RangeXYZ.X < 0 || RangeXYZ.Y < 0 || RangeXYZ.Z < 0)
		return;
	TArray<FBlockInfo> SyncData;
	FVector BlockMin = TransformWorldToBlock(Center - RangeXYZ);
	FVector BlockMax = TransformWorldToBlock(Center + RangeXYZ);
	for (int z = BlockMin.Z; z <= BlockMax.Z; z++)
		for (int y = BlockMin.Y; y <= BlockMax.Y; y++)
			for (int x = BlockMin.X; x <= BlockMax.X; x++)
			{
				//ShouldIncludeEdge Is Now Ignored.
				SyncData.Append(BreakBlock_Internal(FVector(x, y, z)));
			}
	if (SyncData.Num())
		SyncMap_Rep(SyncData);
}

void ACircleOnlyMapGenerator::HitBlock_Implementation(FVector BlockLocation, float Damage)
{
	TArray<FBlockInfo> SyncData;
	BlockLocation = TransformWorldToBlock(BlockLocation);
	SyncData.Append(HitBlock_Internal(BlockLocation, Damage));
	if (SyncData.Num())
		SyncMap_Rep(SyncData);
}

void ACircleOnlyMapGenerator::HitBlockWithNormal_Implementation(FVector BlockLocation, FVector Normal, float Damage)
{
	TArray<FBlockInfo> SyncData;
	BlockLocation = TransformWorldToBlockWithNormal(BlockLocation,Normal);
	SyncData.Append(HitBlock_Internal(BlockLocation, Damage));
	if (SyncData.Num())
		SyncMap_Rep(SyncData);
}

bool ACircleOnlyMapGenerator::IsLocationHaveBlock_Implementation(FVector BlockLocation)
{
	BlockLocation = TransformWorldToBlock(BlockLocation);
	return IsLocationHaveBlock_Internal(BlockLocation);
}

void ACircleOnlyMapGenerator::BreakBlock_Implementation(FVector BlockLocation)
{
	TArray<FBlockInfo> SyncData;
	BlockLocation = TransformWorldToBlock(BlockLocation);
	SyncData.Append(BreakBlock_Internal(BlockLocation));
	if (SyncData.Num())
		SyncMap_Rep(SyncData);
}
