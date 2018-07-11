// Fill out your copyright notice in the Description page of Project Settings.

#include "SkillBase.h"
#include "XBallBase.h"
#include "TimerManager.h"
#include "Engine/World.h"


void ASkillBase::SkillLeave()
{
	GetHolderPawn()->NotifySkillLeave();
}


void ASkillBase::UpdateCoolDown(float CoolDownTime)
{
	if (CoolDownTime <= 0)
	{
		FTimerHandle timeHandleTmp;
		GetWorld()->GetTimerManager().SetTimer(timeHandleTmp, [=]()
			{
				if (IsPendingKill())
					return;
				CoolDown -= 0.02f;
				if (CoolDown<=0)
				{
					CoolDown = 0;
					FTimerHandle timerHandle = timeHandleTmp;
					GetWorld()->GetTimerManager().ClearTimer(timerHandle);
				}
			},0.02f,true);
	}
	else
		CoolDown += CoolDownTime;
}

bool ASkillBase::IsCoolingDown()
{
	return CoolDown > 0;
}

void ASkillBase::SelectedWhileCD_Implementation()
{
	//TwinkleIcon();
}