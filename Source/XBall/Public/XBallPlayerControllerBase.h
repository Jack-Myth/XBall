// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "IMapGenerator.h"
#include "ActionBase.h"
#include "XBallBase.h"
#include "XBallPlayerState.h"
#include "XBallPlayerControllerBase.generated.h"

/**
 * 
 */
UCLASS()
class XBALL_API AXBallPlayerControllerBase : public APlayerController
{
	GENERATED_BODY()


protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		TArray<TSubclassOf<AActionBase>> ActionClasses;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		UClass* PlayerDefaultCharacter;
	UPROPERTY(BlueprintReadOnly,Replicated)
		int Team=0;
public:	
	AXBallPlayerControllerBase();

	UFUNCTION(BlueprintCallable, NetMulticast,Reliable)
		void SetTeam(int NewTeam);
	UFUNCTION(BlueprintCallable)
		int GetTeam();
	UFUNCTION(BlueprintCallable,NetMulticast,Reliable,WithValidation)
		void SetPlayerDefaultCharacter(TSubclassOf<AXBallBase> DefaultCharacter);
	UFUNCTION(BlueprintCallable,NetMulticast,Reliable,WithValidation)
		void SetActionClass(int Index, TSubclassOf<AActionBase> NewActionClass);
	
	inline UClass * GetPlayerDefaultCharacter()
	{
		return PlayerDefaultCharacter;
	}
	inline TArray<TSubclassOf<AActionBase>> GetActionClasses()
	{
		return ActionClasses;
	}
	inline TSubclassOf<AActionBase> GetActionClass(int Index)
	{
		if (Index < 0 || Index >= ActionClasses.Num())
			return nullptr;
		return ActionClasses[Index];
	}

	/*UFUNCTION(BlueprintCallable,Client, Reliable, WithValidation)
		void BuildMap(UClass* MapGenerator, int MaxEngth, int MaxWidth, int MaxHeight,int32 NoiseSeed);*/
	UFUNCTION(BlueprintCallable,Client, Reliable, WithValidation)
		void ReGenOldMap(UClass* MapGenerator,int MaxEngth, int MaxWidth, int MaxHeight, int32 Seed,const TArray<FBlockInfo>& BlockInfo);
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const override;

	virtual void Possess(APawn* aPawn) override;

	UFUNCTION(BlueprintCallable, Server, Reliable,WithValidation)
		void UpdatePicData(const FString& TextureParamName,const TArray<uint8>& TextureData);

	UFUNCTION(BlueprintPure)
	inline class AXBallPlayerState* GetXBallPlayerState()
	{
		return Cast<AXBallPlayerState>(PlayerState);
	}

};
