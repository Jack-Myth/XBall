// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionBase.h"




/*void UActionComponentBase::BeginReady_Implementation(FVector TargetLocation)
{
	BeginReady_Func(TargetLocation);
}

bool UActionComponentBase::BeginAction_Validate(FVector TargetLocation)
{
	return true;
}

bool UActionComponentBase::BeginReady_Validate(FVector TargetLocation)
{
	return true;
}

void UActionComponentBase::BeginAction_Implementation(FVector TargetLocation)
{
	BeginAction_Func(TargetLocation);
}

void UActionComponentBase::EndReady_Implementation(FVector TargetLocation)
{
	EndReady_Func(TargetLocation);
}

bool UActionComponentBase::EndReady_Validate(FVector TargetLocation)
{
	return true;
}*/

AActionBase::AActionBase()
{
	SetReplicates(true);
}

