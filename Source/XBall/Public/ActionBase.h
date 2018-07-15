// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ActionBase.generated.h"

UENUM(BlueprintType)
enum class EActionType:uint8
{
	AT_Weapon		UMETA(Weapon),
	AT_Skill		UMETA(Skill)
};

/**
 * 
 */
UCLASS(abstract)
class XBALL_API AActionBase : public AActor
{
	GENERATED_BODY()

	class AXBallBase* HolderPawn;
protected:
	UPROPERTY(EditDefaultsOnly)
		int Price = 0;
	UPROPERTY(EditDefaultsOnly)
		UTexture2D* ActionIcon;
public:
	AActionBase();
	/*UFUNCTION(BlueprintCallable,Server,Reliable,WithValidation)
		void BeginReady(FVector TargetLocation);
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation)
		void EndReady(FVector TargetLocation);
	UFUNCTION(BlueprintCallable,Server, Reliable,WithValidation)
		void BeginAction(FVector TargetLocation);
	UFUNCTION(BlueprintCallable,Server, Reliable,WithValidation)
		void EndAction(FVector TargetLocation);*/

	void SetHolderPawn(class AXBallBase* HolderPawn)
	{
		this->HolderPawn = HolderPawn;
	}
	UFUNCTION(BlueprintCallable,BlueprintPure)
	class AXBallBase* GetHolderPawn()
	{
		return HolderPawn;
	}
		
	UFUNCTION(BlueprintCallable)
	inline int GetPrice()
	{
		return Price;
	}	

	UFUNCTION(BlueprintCallable)
		inline UTexture2D* GEtActionIcon()
	{
		return ActionIcon;
	}

	UFUNCTION(BlueprintImplementableEvent)
		void BeginSelected(FVector TargetLocation);
	UFUNCTION(BlueprintImplementableEvent)
		void EndSelected(FVector TargetLocation);
	UFUNCTION(BlueprintImplementableEvent)
		void UpdateTargetLocation(FVector TargetLocation);
};
