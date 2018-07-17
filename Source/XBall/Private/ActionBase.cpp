// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionBase.h"
#include "UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "XBallPlayerControllerBase.h"



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

float AActionBase::GetProgressValue_Implementation()
{
	return 1.f;
}

void AActionBase::TwinkleIcon()
{
	auto* XBallController = Cast<AXBallPlayerControllerBase>(UGameplayStatics::GetPlayerController(this, 0));
	if (XBallController)
	{
		UUserWidget* AttachedWidget = XBallController->FindActionBarItemWidgetFor(this);
		if (AttachedWidget)
		{
			AttachedWidget->ProcessEvent(AttachedWidget->FindFunction("TwinkleIcon"), nullptr);
		}
	}
}

