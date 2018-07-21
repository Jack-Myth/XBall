// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "IMapGenerator.h"
#include "MyBPFuncLib.generated.h"

DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnCreateSessionFinished, FName, SessionName, bool, IsSucceed);

/**
 * 
 */
UCLASS()
class XBALL_API UMyBPFuncLib : public UBlueprintFunctionLibrary
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

	UFUNCTION(BlueprintCallable)
		static UTexture2D* GetTextureFromData(const TArray<uint8>& TextureData);
	UFUNCTION(BlueprintCallable)
		static UTexture2D* GetLocalTexture(const FString &_TexPath);
	static TArray<FColor> uint8ToFColor(const TArray<uint8> origin);
	static UTexture2D* TextureFromImage(const int32 SrcWidth, const int32 SrcHeight, const TArray<FColor>&SrcData, const bool UseAlpha);

	UFUNCTION(BlueprintPure)
		static UObject* LoadObjectNative(UObject* Outer, FString ObjectReference);

	UFUNCTION(BlueprintPure)
		static UClass* LoadClassNative(UObject* Outer, FString ClassPath);

	UFUNCTION(BlueprintPure)
		static UTexture2D* GetActionPreview(TSubclassOf<AActionBase> ActionBaseClass);

	UFUNCTION(BlueprintPure)
		static void GetActionInfo(TSubclassOf<AActionBase> ActionClass, FString& Title, FString& Intro, int& Price);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObj"))
		static bool CreateOnlineSessionWithName(UObject* WorldContextObj,class APlayerController* HostedController, int PublicConnections, bool UsingLAN, FName ServerName, FOnCreateSessionFinished OnCreateSessionFinished);
};
