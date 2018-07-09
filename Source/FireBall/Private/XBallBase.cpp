// Fill out your copyright notice in the Description page of Project Settings.

#include "XBallBase.h"
#include "SkillBase.h"
#include "WeaponBase.h"
#include "Components/CapsuleComponent.h"
#include "ConstructorHelpers.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Materials/Material.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "XBallGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "XBallPlayerControllerBase.h"


FVector AXBallBase::GetCursorLocation(FVector* outSurfaceNormal) 
{
	APlayerController* mPlayerController = Cast<APlayerController>(GetController());
	if (mPlayerController)
	{
		FHitResult CursorHitresult;
		mPlayerController->GetHitResultUnderCursorByChannel(ETraceTypeQuery::TraceTypeQuery1, false, CursorHitresult);
		if (outSurfaceNormal)
			*outSurfaceNormal = CursorHitresult.ImpactNormal;
		if (CursorHitresult.bBlockingHit)
		{
			return CursorHitresult.Location;
		}
	}
	return FVector(0, 0, 0);
}

// Sets default values
AXBallBase::AXBallBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Setup Socket
	// Setup WeaponSocket
	// No Socket Needed.
	//WeaponSocket = CreateDefaultSubobject<USceneComponent>("WeaponSocket");
	//WeaponSocket->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	//WeaponSocket->SetRelativeLocation(FVector(0, 90, 0));
	//// Setup SkillSocket
	//SkillSocket = CreateDefaultSubobject<USceneComponent>("SkillSocket");
	//SkillSocket->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	//SkillSocket->SetRelativeLocation(FVector(0, -90, 0));

	GetCapsuleComponent()->SetCapsuleHalfHeight(50, true);
	GetCapsuleComponent()->SetCapsuleRadius(50, true);
	CoreBallMesh = CreateDefaultSubobject<UStaticMeshComponent>("CoreBallMeshComponent");
	CoreBallMesh->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	CoreBallMesh->SetRelativeLocation(FVector(0, 0, 0));
	CoreBallMesh->SetStaticMesh(ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'")).Object);
	bUseControllerRotationPitch = true;
	bUseControllerRotationRoll = true;
	//Yaw has been setted Defaultly;
	// Setup Camera
	CameraArm = CreateDefaultSubobject<USpringArmComponent>("CameraArm");
	CameraArm->SetAbsolute(false, true);
	CameraArm->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	CameraArm->SetRelativeLocation(FVector(0.f));
	CameraArm->SetWorldRotation(FRotator(-50.f, 0.f, 0));
	CameraArm->TargetArmLength = 600;
	PlayerCamera = CreateDefaultSubobject<UCameraComponent>("PlayerCamera");
	PlayerCamera->AttachToComponent(CameraArm, FAttachmentTransformRules::KeepRelativeTransform);
	PlayerCamera->SetRelativeLocation(FVector(0.f));

	// Enable Replication
	SetReplicates(true);
	SetReplicateMovement(true);

	// Set Default Movement Details
	JumpMaxHoldTime = 0.3f;
	JumpMaxCount = 2;
	GetCharacterMovement()->AirControl = 0.7f;
	
}

void AXBallBase::UpdatePlayerTarget()
{
	if (!PlayerTargetClass||PlayerTarget&&PlayerTarget->GetClass() == PlayerTargetClass)
		return;
	UObject* NewTarget;
	if (PlayerTargetClass->IsChildOf<AActor>())
	{
		NewTarget = GetWorld()->SpawnActor<AActor>(PlayerTargetClass, FTransform::Identity);
	}
	else if (PlayerTargetClass->IsChildOf<UPrimitiveComponent>())
	{
		NewTarget = NewObject<UPrimitiveComponent>(this, PlayerTargetClass);
		((UPrimitiveComponent*)NewTarget)->RegisterComponent();
	}
	else
	{
		NewTarget = NewObject<UObject>(this, PlayerTargetClass);
	}
	if (!NewTarget)
		return;
	if(::IsValid(PlayerTarget))
		IScenePlayerTarget::Execute_NotifyDestruction(PlayerTarget);
	//CameraArm->SetWorldRotation(FRotator(-50, 0, 0));
	PlayerTarget = NewTarget;
	FVector SurfaceNormal;
	FVector CursorPos = GetCursorLocation(&SurfaceNormal);
	IScenePlayerTarget::Execute_OnUpdateTargetPosition(PlayerTarget, CursorPos,SurfaceNormal);
}

void AXBallBase::SetHealth(int NewHealth)
{
	Health = NewHealth;
	CheckShouldDie();
}

int AXBallBase::GetHealth()
{
	return Health;
}


void AXBallBase::NotifySkillLeave()
{
	CurrentSkill = nullptr;
}

void AXBallBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if(PlayerTarget)
		IScenePlayerTarget::Execute_NotifyDestruction(PlayerTarget);
}

void AXBallBase::CheckShouldDie_Implementation()
{
	if (Health <= 0)
	{
		DieDefault();
	}
}

void AXBallBase::NotifyRespawn()
{
	AXBallGameModeBase* pGameMode = Cast<AXBallGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (pGameMode) //Client May Call This Function, Then Will get invalid GameMode.
	{
		pGameMode->PrepareReSpawn((AXBallPlayerControllerBase*)GetController());
	}
}

void AXBallBase::DieDefault_Implementation()
{
	//Only Server Can Respawn Player Character.
	if(HasAuthority())
		NotifyRespawn();

	//Should Spawn an Emitter'
	this->Destroy();
}

// Called when the game starts or when spawned
void AXBallBase::BeginPlay()
{
	Super::BeginPlay();

	// Setup Cursor£¨Player Target£©
	UpdatePlayerTarget();
}

void AXBallBase::UpdateRotation_Implementation(FVector TargetLocation)
{
	UpdateRotation_Internal(TargetLocation);
}


bool AXBallBase::UpdateRotation_Validate(FVector TargetLocation)
{
	return true;
}

void AXBallBase::UpdateRotation_Internal(FVector TargetLocation)
{
	FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TargetLocation);
	AController* mController = GetController();
	if (mController&&mController->IsLocalController())
	{
		mController->SetControlRotation(TargetRotation);
	}
	if (CurrentWeapon)
	{
		CurrentWeapon->UpdateTargetLocation(TargetLocation);
	}
	if (CurrentSkill)
	{
		CurrentSkill->UpdateTargetLocation(TargetLocation);
	}
}

void AXBallBase::Input_MoveForward(float AxisValue)
{
	AddMovementInput(FVector(1,0,0), AxisValue);
}

void AXBallBase::Input_MoveRight(float AxisValue)
{
	AddMovementInput(FVector(0, 1, 0), AxisValue);
}

void AXBallBase::Input_MouseX(float AxisValue)
{
	FVector SurfaceNormal;
	FVector TargetLocation = GetCursorLocation(&SurfaceNormal);
	if (TargetLocation == TargetLocationCache)
		return;
	TargetLocationCache = TargetLocation;
	if (PlayerTarget)
		IScenePlayerTarget::Execute_OnUpdateTargetPosition(PlayerTarget, TargetLocation, SurfaceNormal);
	UpdateRotation(TargetLocation);
	UpdateRotation_Internal(TargetLocation);
	if (!IsLocallyControlled())
		UpdateRotation(TargetLocation);
}

void AXBallBase::Input_MouseY(float AxisValue)
{
}

void AXBallBase::Input_JumpStart()
{
	Jump();
}

void AXBallBase::Input_JumpEnd()
{
	StopJumping();
}

void AXBallBase::Input_BeginFire()
{
	BeginFireWeapon(GetCursorLocation());
}

void AXBallBase::Input_EndFire()
{
	EndFireWeapon(GetCursorLocation());
}

void AXBallBase::Input_BeginAction1()
{
	BeginSelectAction(1, GetCursorLocation());
}

void AXBallBase::Input_EndAction1()
{
	EndSelectAction(1,GetCursorLocation());
}

void AXBallBase::Input_BeginMainAction()
{
	BeginSelectAction(0, GetCursorLocation());
}

void AXBallBase::Input_EndMainAction()
{
	EndSelectAction(0, GetCursorLocation());
}
 
void AXBallBase::Input_BeginAction2()
{
	BeginSelectAction(2, GetCursorLocation());
}

void AXBallBase::Input_EndAction2()
{
	EndSelectAction(2, GetCursorLocation());
}

void AXBallBase::Input_BeginAction3()
{
	BeginSelectAction(3, GetCursorLocation());
}

void AXBallBase::Input_EndAction3()
{
	EndSelectAction(3, GetCursorLocation());
}

void AXBallBase::Input_BeginAction4()
{
	BeginSelectAction(4, GetCursorLocation());
}

void AXBallBase::Input_EndAction4()
{
	EndSelectAction(4, GetCursorLocation());
}

void AXBallBase::Input_BeginAction5()
{
	BeginSelectAction(5, GetCursorLocation());
}

void AXBallBase::Input_EndAction5()
{
	EndSelectAction(5, GetCursorLocation());
}

void AXBallBase::Input_BeginAction6()
{
	BeginSelectAction(6, GetCursorLocation());
}

void AXBallBase::Input_EndAction6()
{
	EndSelectAction(6, GetCursorLocation());
}

void AXBallBase::Input_BeginAction7()
{
	BeginSelectAction(7, GetCursorLocation());
}

void AXBallBase::Input_EndAction7()
{
	EndSelectAction(7, GetCursorLocation());
}

void AXBallBase::EndFireWeapon_Implementation(FVector TargetLocation)
{
	if (CurrentWeapon)
		CurrentWeapon->EndFire(TargetLocation);
}

bool AXBallBase::EndFireWeapon_Validate(FVector TargetLocation)
{
	return true;
}

void AXBallBase::BeginFireWeapon_Implementation(FVector TargetLocation)
{
	if (CurrentWeapon)
		CurrentWeapon->BeginFire(TargetLocation);
}

bool AXBallBase::BeginFireWeapon_Validate(FVector TargetLocation)
{
	return true;
}

void AXBallBase::BeginSelectAction_Implementation(int ActionIndex, FVector TargetLocation)
{
	AXBallPlayerControllerBase* mController = Cast<AXBallPlayerControllerBase>(GetController());
	if (!mController)
	{
		UE_LOG(LogTemp, Error, TEXT("XBallBase Possess by Controller That Not a AXBallPlayerController"));
		return;
	}
	UClass* TargetClass = mController->GetActionClass(ActionIndex);
	if (!TargetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid Action Class"));
		return;
	}
	if(!(::IsValid(CurrentSkill))&&TargetClass->IsChildOf(ASkillBase::StaticClass()))
	{
		UE_LOG(LogTemp, Display, TEXT("Reselect Skill"));
		CurrentSkill = Cast<ASkillBase>(GetWorld()->SpawnActor(TargetClass));
		CurrentSkill->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
		CurrentSkill->SetActorRelativeLocation(FVector(0, -60, 0));
		//CurrentSkill->AttachToComponent(SkillSocket, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		CurrentSkill->GetRootComponent()->SetVisibility(false, true);
		CurrentSkill->SetHolderPawn(this);
		CurrentSkill->BeginSelected(TargetLocation);
	}
	else if (TargetClass->IsChildOf(AWeaponBase::StaticClass()))
	{
		if (CurrentWeapon)
		{
			CurrentWeapon->OnSwitched();
		}
		CurrentWeapon = Cast<AWeaponBase>(GetWorld()->SpawnActor(TargetClass));
		CurrentWeapon->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
		CurrentWeapon->SetActorRelativeLocation(FVector(0, 60, 0));
		//CurrentWeapon->AttachToComponent(WeaponSocket, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		CurrentWeapon->GetRootComponent()->SetVisibility(false, true);
		CurrentWeapon->BeginSelected(TargetLocation);
	}
}

bool AXBallBase::BeginSelectAction_Validate(int ActionIndex, FVector TargetLocation)
{
	return true;
}

void AXBallBase::EndSelectAction_Implementation(int ActionIndex, FVector TargetLocation)
{
	AXBallPlayerControllerBase* mController = Cast<AXBallPlayerControllerBase>(GetController());
	if (!mController)
	{
		UE_LOG(LogTemp, Error, TEXT("XBallBase Possess by Controller That Not a AXBallPlayerController"));
		return;
	}
	UClass* TargetClass = mController->GetActionClass(ActionIndex);
	if (!TargetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid Action Class"));
		return;
	}
	if (TargetClass->IsChildOf(ASkillBase::StaticClass())&&CurrentSkill)
	{
		CurrentSkill->EndSelected(TargetLocation);
		//CurrentSkill shouldn't be set to nullptr, NotifySkillLeave() will do this job.
		//CurrentSkill = nullptr;
	}
	else if (TargetClass->IsChildOf(AWeaponBase::StaticClass()) &&CurrentWeapon)
	{
		CurrentWeapon->EndSelected(TargetLocation);
	}
}

bool AXBallBase::EndSelectAction_Validate(int ActionIndex, FVector TargetLocation)
{
	return true;
}

// Called every frame
void AXBallBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	//CameraArm->SetWorldRotation(FRotator(-50, 0, 0));
}

// Called to bind functionality to input
void AXBallBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &AXBallBase::Input_MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AXBallBase::Input_MoveRight);
	PlayerInputComponent->BindAxis("MouseXAxis", this, &AXBallBase::Input_MouseX);
	PlayerInputComponent->BindAxis("MouseYAxis", this, &AXBallBase::Input_MouseY);
	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &AXBallBase::Input_JumpStart);
	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Released, this, &AXBallBase::Input_JumpEnd);
	PlayerInputComponent->BindAction("Fire", EInputEvent::IE_Pressed, this, &AXBallBase::Input_BeginFire);
	PlayerInputComponent->BindAction("Fire", EInputEvent::IE_Released, this, &AXBallBase::Input_EndFire);
	PlayerInputComponent->BindAction("MainAction", EInputEvent::IE_Pressed, this, &AXBallBase::Input_BeginMainAction);
	PlayerInputComponent->BindAction("MainAction", EInputEvent::IE_Released, this, &AXBallBase::Input_EndMainAction);
	PlayerInputComponent->BindAction("Action1", EInputEvent::IE_Pressed, this, &AXBallBase::Input_BeginAction1);
	PlayerInputComponent->BindAction("Action1", EInputEvent::IE_Released, this, &AXBallBase::Input_EndAction1);
	PlayerInputComponent->BindAction("Action2", EInputEvent::IE_Pressed, this, &AXBallBase::Input_BeginAction2);
	PlayerInputComponent->BindAction("Action2", EInputEvent::IE_Released, this, &AXBallBase::Input_EndAction2);
	PlayerInputComponent->BindAction("Action3", EInputEvent::IE_Pressed, this, &AXBallBase::Input_BeginAction3);
	PlayerInputComponent->BindAction("Action3", EInputEvent::IE_Released, this, &AXBallBase::Input_EndAction3);
	PlayerInputComponent->BindAction("Action4", EInputEvent::IE_Pressed, this, &AXBallBase::Input_BeginAction4);
	PlayerInputComponent->BindAction("Action4", EInputEvent::IE_Released, this, &AXBallBase::Input_EndAction4);
	PlayerInputComponent->BindAction("Action5", EInputEvent::IE_Pressed, this, &AXBallBase::Input_BeginAction5);
	PlayerInputComponent->BindAction("Action5", EInputEvent::IE_Released, this, &AXBallBase::Input_EndAction5);
	PlayerInputComponent->BindAction("Action6", EInputEvent::IE_Pressed, this, &AXBallBase::Input_BeginAction6);
	PlayerInputComponent->BindAction("Action6", EInputEvent::IE_Released, this, &AXBallBase::Input_EndAction6);
	PlayerInputComponent->BindAction("Action7", EInputEvent::IE_Pressed, this, &AXBallBase::Input_BeginAction7);
	PlayerInputComponent->BindAction("Action7", EInputEvent::IE_Released, this, &AXBallBase::Input_EndAction7);
}

void AXBallBase::InitPlayer_Implementation(int Team)
{
	switch (Team)
	{
		case 1:
			CoreBallMesh->SetMaterial(0,LoadObject<UMaterial>(nullptr, TEXT("MaterialInstanceConstant'/Game/FireBall/Materials/TeamMat/TeamColorBase_Blue.TeamColorBase_Blue'")));
			break;
		case 2:
			CoreBallMesh->SetMaterial(0,LoadObject<UMaterial>(nullptr, TEXT("MaterialInstanceConstant'/Game/FireBall/Materials/TeamMat/TeamColorBase_Red.TeamColorBase_Red'")));
			break;
	}
}

bool AXBallBase::InitPlayer_Validate(int Team)
{
	return true;
}
