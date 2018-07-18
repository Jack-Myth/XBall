// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionBase.h"
#include "WeaponBase.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class XBALL_API AWeaponBase : public AActionBase
{
	GENERATED_BODY()

public: 

	UPROPERTY(BlueprintReadOnly,EditDefaultsOnly)
		int MaxAmmo=1;

	UPROPERTY(BlueprintReadWrite,Replicated)
		int CurrentAmmo;

	UPROPERTY(EditDefaultsOnly)
		float ReloadTime=3;

	UPROPERTY(Replicated)
		float ReloadTimeRemain;
	FTimerHandle ReloadTimerHandle;

	// Reload Weapon,It Will Set CurrentAmmo To MaxAmmo;
	UFUNCTION(BlueprintCallable)
		void Reload();

	UFUNCTION(BlueprintPure)
		inline bool IsReloading()
	{
		return ReloadTimeRemain > 0;
	}

	UFUNCTION(BlueprintImplementableEvent)
		void BeginFire(FVector TargetLocation);

	UFUNCTION(BlueprintImplementableEvent)
		void EndFire(FVector TargetLocation);

	UFUNCTION(BlueprintImplementableEvent)
		void OnSwitched();

	UFUNCTION(BlueprintPure)
		virtual float GetProgressValue_Implementation() override;
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

};
