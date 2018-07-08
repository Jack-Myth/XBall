// Fill out your copyright notice in the Description page of Project Settings.

#include "SkillBase.h"
#include "XBallBase.h"


void ASkillBase::SkillLeave()
{
	GetHolderPawn()->NotifySkillLeave();
}
