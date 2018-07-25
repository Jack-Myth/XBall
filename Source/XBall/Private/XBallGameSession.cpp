// Fill out your copyright notice in the Description page of Project Settings.

#include "XBallGameSession.h"
#include "Kismet/GameplayStatics.h"
#include "XBallGameInstanceNative.h"




void AXBallGameSession::PostLogin(APlayerController* NewPlayer)
{
	TArray<AActor*> PlayerControllers;
	UGameplayStatics::GetAllActorsOfClass(this, APlayerController::StaticClass(), PlayerControllers);
	if (PlayerControllers.Num()>Cast<UXBallGameInstanceNative>(UGameplayStatics::GetGameInstance(this))->MaxConnection)
	{
		KickPlayer(NewPlayer, FText::FromString("The Server is Full!"));
	}
}
