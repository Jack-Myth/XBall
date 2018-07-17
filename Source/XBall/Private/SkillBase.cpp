// Fill out your copyright notice in the Description page of Project Settings.

#include "SkillBase.h"
#include "XBallBase.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "UnrealNetwork.h"


void ASkillBase::SkillLeave()
{
	GetHolderPawn()->NotifySkillLeave();
}


bool ASkillBase::UpdateCoolDown_Validate(float CoolDownTime)
{
	return true;
}

void ASkillBase::UpdateCoolDown_Implementation(float CoolDownTime)
{
	GetWorld()->GetTimerManager().ClearTimer(CoolDownTimeHandle);
	CoolDown += CoolDownTime;
	GetWorld()->GetTimerManager().SetTimer(CoolDownTimeHandle, [this]()
		{
			if (IsPendingKill())
				return;
			CoolDown -= 0.02f;
			if (CoolDown <= 0)
			{
				CoolDown = 0;
				GetWorld()->GetTimerManager().ClearTimer(CoolDownTimeHandle);
			}
		}, 0.02f, true);
}

void ASkillBase::SelectedWhileCD_Implementation()
{
	TwinkleIcon();
}

bool ASkillBase::IsCoolingDown()
{
	return CoolDown > 0;
}

float ASkillBase::GetCoolDownRemain()
{
	return CoolDown;
}

void ASkillBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASkillBase, CoolDown);
}

float ASkillBase::GetProgressValue_Implementation()
{
	return FMath::Clamp<float>(1.f - CoolDown / MaxCoolDown, 0, 1);
}
