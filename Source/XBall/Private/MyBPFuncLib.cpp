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
#include "FileHelper.h"
#include "Engine/Texture2D.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"

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

UTexture2D* UMyBPFuncLib::GetTextureFromData(const TArray<uint8>& TextureData)
{
	UTexture2D* OutTex = NULL;
	IImageWrapperModule& imageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> imageWrapper = imageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);

	if (TextureData.Num())
	{
		if (imageWrapper.IsValid() &&
			imageWrapper->SetCompressed(TextureData.GetData(), TextureData.Num()))
		{
			const TArray<uint8>* uncompressedRGBA = NULL;
			if (imageWrapper->GetRaw(ERGBFormat::RGBA, 8, uncompressedRGBA))
			{
				const TArray<FColor> uncompressedFColor = uint8ToFColor(*uncompressedRGBA);
				OutTex = TextureFromImage(
					imageWrapper->GetWidth(),
					imageWrapper->GetHeight(),
					uncompressedFColor,
					true);
			}
		}
	}
	return OutTex;
}

UTexture2D* UMyBPFuncLib::GetLocalTexture(const FString &_TexPath)
{
	UTexture2D* OutTex = NULL;
	TArray<uint8> OutArray;
	if (FFileHelper::LoadFileToArray(OutArray, *_TexPath))
		OutTex = GetTextureFromData(OutArray);
	return OutTex;
}

TArray<FColor> UMyBPFuncLib::uint8ToFColor(const TArray<uint8> origin)
{
	TArray<FColor> uncompressedFColor;
	uint8 auxOrigin;
	FColor auxDst;

	for (int i = 0; i < origin.Num(); i++)
	{
		auxOrigin = origin[i];
		auxDst.R = auxOrigin;
		i++;
		auxOrigin = origin[i];
		auxDst.G = auxOrigin;
		i++;
		auxOrigin = origin[i];
		auxDst.B = auxOrigin;
		i++;
		auxOrigin = origin[i];
		auxDst.A = auxOrigin;
		uncompressedFColor.Add(auxDst);
	}

	return uncompressedFColor;
}

UTexture2D* UMyBPFuncLib::TextureFromImage(const int32 SrcWidth, const int32 SrcHeight, const TArray<FColor>&SrcData, const bool UseAlpha)
{
	UTexture2D* MyScreenshot = UTexture2D::CreateTransient(SrcWidth, SrcHeight, PF_B8G8R8A8);
	uint8* MipData = static_cast<uint8*>(MyScreenshot->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE));
	uint8* DestPtr = NULL;
	const FColor* SrcPtr = NULL;
	for (int32 y = 0; y < SrcHeight; y++)
	{
		DestPtr = &MipData[(SrcHeight - 1 - y)*SrcWidth * sizeof(FColor)];
		SrcPtr = const_cast<FColor*>(&SrcData[(SrcHeight - 1 - y)*SrcWidth]);
		for (int32 x = 0; x < SrcWidth; x++)
		{
			*DestPtr++ = SrcPtr->B;
			*DestPtr++ = SrcPtr->G;
			*DestPtr++ = SrcPtr->R;
			if (UseAlpha)
			{
				*DestPtr++ = SrcPtr->A;
			}
			else
			{
				*DestPtr++ = 0xFF;
			}
			SrcPtr++;
		}
	}

	MyScreenshot->PlatformData->Mips[0].BulkData.Unlock();
	MyScreenshot->UpdateResource();

	return MyScreenshot;
}