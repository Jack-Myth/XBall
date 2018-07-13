// Fill out your copyright notice in the Description page of Project Settings.

#include "DefaultPlayerTarget.h"
#include "ConstructorHelpers.h"
#include "Materials/Material.h"
#include "Components/DecalComponent.h"
#include "Kismet/KismetMathLibrary.h"


ADefaultPlayerTarget::ADefaultPlayerTarget()
{
	SetDecalMaterial(ConstructorHelpers::FObjectFinder<UMaterial>(TEXT("Material'/Game/XBall/Materials/DefaultTarget.DefaultTarget'")).Object);
	GetDecal()->DecalSize = FVector(10, 50, 50);
}

void ADefaultPlayerTarget::OnUpdateTargetPosition_Implementation(FVector WorldLocation, FVector SurfaceNormal)
{
	SetActorLocation(WorldLocation);
	SetActorRotation(UKismetMathLibrary::FindLookAtRotation(SurfaceNormal, FVector(0, 0, 0)));
}

void ADefaultPlayerTarget::NotifyDestruction_Implementation()
{
	Destroy();
}
