// Fill out your copyright notice in the Description page of Project Settings.

#include "WeaponBase.h"
#include "UnrealNetwork.h"




void AWeaponBase::Reload()
{
	if (ReloadTimeRemain<=0)
	{
		ReloadTimeRemain = ReloadTime;
		GetWorld()->GetTimerManager().SetTimer(ReloadTimerHandle, [this]()
			{
				ReloadTimeRemain -= 0.02f;
				if (ReloadTimeRemain<=0)
				{
					CurrentAmmo = MaxAmmo;
					GetWorld()->GetTimerManager().ClearTimer(ReloadTimerHandle);
				}
			}, 0.02f, true);
	}
}

float AWeaponBase::GetProgressValue_Implementation()
{
	if (IsReloading())
	{
		return 1 - (ReloadTimeRemain / ReloadTime);
	}
	else
	{
		return (float)CurrentAmmo / (float)MaxAmmo;
	}
}

void AWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWeaponBase, CurrentAmmo);
	DOREPLIFETIME(AWeaponBase, ReloadTimeRemain);
}

void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	CurrentAmmo = MaxAmmo;
}
