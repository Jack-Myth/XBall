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
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"
#include "Engine/Texture2D.h"
#include "UnrealNetwork.h"


FVector AXBallBase::GetCursorLocation(FVector* outSurfaceNormal) 
{
	APlayerController* mPlayerController = Cast<APlayerController>(GetController());
	if (mPlayerController)
	{
		FHitResult CursorHitresult;
		mPlayerController->GetHitResultUnderCursorByChannel(ETraceTypeQuery::TraceTypeQuery1, false, CursorHitresult);
		if (CursorHitresult.GetActor()==this)
		{
			UKismetSystemLibrary::LineTraceSingle(this, PlayerCamera->GetComponentLocation(),
				(CursorHitresult.Location - PlayerCamera->GetComponentLocation()) * 2 + PlayerCamera->GetComponentLocation(), ETraceTypeQuery::TraceTypeQuery1,
				false, TArray<AActor*>({ this }), EDrawDebugTrace::None, CursorHitresult, true);
		}
		if (outSurfaceNormal)
			*outSurfaceNormal = CursorHitresult.ImpactNormal;
		if (CursorHitresult.bBlockingHit)
		{
			return CursorHitresult.Location;
		}
	}
	return FVector(0, 0, 0);
}

void AXBallBase::Sprint_Implementation(FVector Dir)
{
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, "Try Sprint");
	if (IsSprintable())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, "Sprint!");
		Dir.Normalize();
		if (GetCharacterMovement()->IsFalling())
		{
			LaunchCharacter(Dir * 2000, true, false);
		}
		else
		{
			LaunchCharacter(Dir * 5000, true, false);
		}
		bSprintable = false;
		SprintCoolDown = 5.f;
		GetWorld()->GetTimerManager().SetTimer(SprintCoolDownTimerHandle, [this]()
			{
				SprintCoolDown -= 0.02f;
				if (SprintCoolDown<=0)
				{
					SprintCoolDown = 0;
					bSprintable = true;
					GetWorld()->GetTimerManager().ClearTimer(SprintCoolDownTimerHandle);
				}
			}, 0.02f, true);
	}
}

bool AXBallBase::Sprint_Validate(FVector Dir)
{
	return true;
}

void AXBallBase::DoReload_Implementation()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->Reload();
	}
}

bool AXBallBase::DoReload_Validate()
{
	return true;
}

FString AXBallBase::GetXBallName_Implementation()
{
	return "XBall";
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
	
	// Setup Take Any Damage Delegate
	OnTakeAnyDamage.AddDynamic(this, &AXBallBase::AnyDamage_Internal);

	ActionList.SetNum(8,false);
	AutoPossessAI = EAutoPossessAI::Disabled;
}

AActionBase* AXBallBase::AddActionToBar(int Index, AActionBase* Action)
{
	if (Index < 0 || Index>7)
		return nullptr;
	AActionBase* LastAction=nullptr;
	if (ActionList[Index])
	{
		LastAction = ActionList[Index];
	}
	ActionList[Index] = Action;
	return LastAction;
}

AActionBase* AXBallBase::RemoveActionFromBar(int Index)
{
	if (Index < 0 || Index>7)
		return nullptr;
	AActionBase* LastAction = ActionList[Index];
	ActionList[Index] = nullptr;
	return LastAction;
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


void AXBallBase::NotifySkillLeave()
{
	CurrentSkill = nullptr;
}

void AXBallBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if(PlayerTarget)
		IScenePlayerTarget::Execute_NotifyDestruction(PlayerTarget);
	for (int i = 0; i < ActionList.Num(); i++)
	{
		if (ActionList[i])
		{
			ActionList[i]->Destroy();
		}
	}
}

void AXBallBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	AXBallPlayerControllerBase* XballController = Cast<AXBallPlayerControllerBase>(NewController);
	if (XballController)
	{
		Team = XballController->GetTeam();
		RefreshPlayerAppearance(Team);
		if (XballController->GetXBallPlayerState())
		{
			XballController->GetXBallPlayerState()->HoldingCharacter = this;
		}
	}
}

void AXBallBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AXBallBase, ActionList);
	DOREPLIFETIME(AXBallBase, Team);
	DOREPLIFETIME(AXBallBase, Health);
	DOREPLIFETIME(AXBallBase, CurrentWeapon);
	DOREPLIFETIME(AXBallBase, CurrentSkill);
	DOREPLIFETIME(AXBallBase, bSprintable);
}

void AXBallBase::CheckShouldDie_Implementation(AController* InstigatedBy)
{
	if (Health <= 0)
	{
		DieDefault(InstigatedBy);
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

void AXBallBase::DieDefault_Implementation(AController* InstigatedBy)
{
	//Only Server Can Respawn Player Character.
	if (HasAuthority())
	{
		NotifyRespawn();
		GetXBallController()->GetXBallPlayerState()->DeadCount++;
		AXBallPlayerControllerBase* XBallController = Cast<AXBallPlayerControllerBase>(InstigatedBy);
		if (XBallController)
		{
			XBallController->GetXBallPlayerState()->KillScore++;
		}
		AXBallGameModeBase* XBallGameMode= Cast<AXBallGameModeBase>(UGameplayStatics::GetGameMode(this));
		if (XBallGameMode)
		{
			XBallGameMode->CheckScore();
		}
	}
	//Should Spawn an Emitter'
	this->Destroy();
}

// Called when the game starts or when spawned
void AXBallBase::BeginPlay()
{
	Super::BeginPlay();
	//Setup Camera Effect
	DamagedEffect = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this, LoadObject<UMaterial>(nullptr, TEXT("Material'/Game/XBall/Materials/DamageEffect.DamageEffect'")));
	PlayerCamera->AddOrUpdateBlendable(DamagedEffect);

	// Setup Cursor（Player Target）
	UpdatePlayerTarget();
}

void AXBallBase::ShowScreenEffectDamaged_Rep_Implementation(int Damage, const AController* Instigater, const class AActor* DamageCauser, const class UDamageType* DamageType)
{
	ShowScreenEffectDamaged(Damage, Instigater, DamageCauser, DamageType);
}

void AXBallBase::ShowScreenEffectDamaged_Implementation(int Damage, const AController* Instigater, const class AActor* DamageCauser, const class UDamageType* DamageType)
{
	GetWorld()->GetTimerManager().ClearTimer(ScreenEffectDamage_TimerHandle);
	DamageBlendAlpha += 0.3f;
	GetWorld()->GetTimerManager().SetTimer(ScreenEffectDamage_TimerHandle, [=]()
		{
			DamageBlendAlpha -= 0.02;
			if (DamageBlendAlpha <= 0)
			{
				DamageBlendAlpha = 0;
				GetWorld()->GetTimerManager().ClearTimer(ScreenEffectDamage_TimerHandle);
			}
			DamagedEffect->SetScalarParameterValue("Alpha", DamageBlendAlpha);
		}, 0.02f, true, -1.f);
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

void AXBallBase::AnyDamage_Internal(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	UE_LOG(LogTemp, Display, TEXT("Pawn Has been Damaged"));
	AXBallPlayerControllerBase* XBallController = Cast<AXBallPlayerControllerBase>(InstigatedBy);
	if (XBallController&&Cast<AXBallGameModeBase>(UGameplayStatics::GetGameMode(this))->IsTeamGame())
	{
		AXBallPlayerControllerBase* SelfXballController = GetXBallController();
		if (!SelfXballController ||
			!(SelfXballController == XBallController) ||
			SelfXballController->GetTeam() != XBallController->GetTeam())
		{
			Health -= Damage;
		}
	}
	else
	{
		Health -= Damage;
	}
	ShowScreenEffectDamaged_Rep((int)Damage, InstigatedBy, DamageCauser, DamageType);
	CheckShouldDie(InstigatedBy);
}

void AXBallBase::Input_MoveForward(float AxisValue)
{
	AddMovementInput(FVector(1,0,0), AxisValue);
	MovementDir.X = AxisValue;
}

void AXBallBase::Input_MoveRight(float AxisValue)
{
	AddMovementInput(FVector(0, 1, 0), AxisValue);
	MovementDir.Y = AxisValue;
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

void AXBallBase::Input_Reload()
{
	DoReload();
}

void AXBallBase::Input_Sprint()
{
	Sprint(MovementDir);
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
	if (ActionIndex < 0 || ActionIndex>7)
		return;
	if(!(::IsValid(CurrentSkill))&&ActionList[ActionIndex]&&ActionList[ActionIndex]->IsA<ASkillBase>())
	{
		ASkillBase* tmpSkill = Cast<ASkillBase>(ActionList[ActionIndex]);
		if (tmpSkill->IsCoolingDown())
		{
			tmpSkill->SelectedWhileCD();
			return;
		}
		CurrentSkill = tmpSkill;
		CurrentSkill->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
		CurrentSkill->SetActorRelativeLocation(FVector(0, -60, 0));
		//CurrentSkill->AttachToComponent(SkillSocket, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		//CurrentSkill->GetRootComponent()->SetVisibility(true, true);
		CurrentSkill->SetHolderPawn(this);
		CurrentSkill->BeginSelected(TargetLocation);
	}
	else if (ActionList[ActionIndex]&&ActionList[ActionIndex]->IsA<AWeaponBase>())
	{
		if (CurrentWeapon)
		{
			CurrentWeapon->OnSwitched();
		}
		CurrentWeapon = Cast<AWeaponBase>(ActionList[ActionIndex]);
		CurrentWeapon->AttachToActor(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		CurrentWeapon->SetActorRelativeLocation(FVector(0, 60, 0));
		CurrentWeapon->SetHolderPawn(this);
		//CurrentWeapon->AttachToComponent(WeaponSocket, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		//CurrentWeapon->GetRootComponent()->SetVisibility(false, true);
		CurrentWeapon->BeginSelected(TargetLocation);
	}
}

bool AXBallBase::BeginSelectAction_Validate(int ActionIndex, FVector TargetLocation)
{
	return true;
}

void AXBallBase::EndSelectAction_Implementation(int ActionIndex, FVector TargetLocation)
{
	if (ActionList[ActionIndex]&&ActionList[ActionIndex]->IsA<ASkillBase>()&&CurrentSkill)
	{
		if (CurrentSkill->IsCoolingDown())
			return;
		//CurrentSkill->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		//CurrentSkill->GetRootComponent()->SetVisibility(false, true);
		CurrentSkill->EndSelected(TargetLocation);
		CurrentSkill = nullptr;
	}
	else if (ActionList[ActionIndex] && ActionList[ActionIndex]->IsA<AWeaponBase>() &&CurrentWeapon)
	{
		//CurrentWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		//CurrentWeapon->GetRootComponent()->SetVisibility(false, true);
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
	PlayerInputComponent->BindAction("Reload", EInputEvent::IE_Pressed, this, &AXBallBase::Input_Reload);
	PlayerInputComponent->BindAction("Sprint", EInputEvent::IE_Pressed, this, &AXBallBase::Input_Sprint);
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

class AXBallPlayerControllerBase* AXBallBase::GetXBallController()
{
	return Cast<AXBallPlayerControllerBase>(GetController());
}

void AXBallBase::SetEntireMaterial_Implementation(UMaterialInterface* Mat)
{
	CoreBallMesh->SetMaterial(0, Mat);
}

void AXBallBase::RefreshPlayerAppearance(int Team)
{
	UE_LOG(LogTemp, Display, TEXT("Init Player"));
	UMaterialInterface* MatInterface;
	switch (Team)
	{
		case 0: //默认
			MatInterface=LoadObject<UMaterial>(nullptr, TEXT("Material'/Game/XBall/Materials/TeamMat/TeamColorBase.TeamColorBase'"));
			break;
		case 1: //红队
			MatInterface = LoadObject<UMaterialInstance>(nullptr, TEXT("MaterialInstanceConstant'/Game/XBall/Materials/TeamMat/TeamColor_Red.TeamColor_Red'"));
			break;
		case 2: //绿队
			MatInterface = LoadObject<UMaterialInstance>(nullptr, TEXT("MaterialInstanceConstant'/Game/XBall/Materials/TeamMat/TeamColor_Green.TeamColor_Green'"));
			break;
		case 3: //蓝队
			MatInterface= LoadObject<UMaterialInstance>(nullptr, TEXT("MaterialInstanceConstant'/Game/XBall/Materials/TeamMat/TeamColor_Blue.TeamColor_Blue'"));
			break;
		case 4: //黄队
			MatInterface= LoadObject<UMaterialInstance>(nullptr, TEXT("MaterialInstanceConstant'/Game/XBall/Materials/TeamMat/TeamColor_Yellow.TeamColor_Yellow'"));
			break;
		default:
			MatInterface= LoadObject<UMaterial>(nullptr, TEXT("Material'/Game/XBall/Materials/TeamMat/TeamColorBase.TeamColorBase'"));
			break;
	}
	TargetDMI = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this, MatInterface);
	if (GetXBallController())
	{
		AXBallPlayerState* PlayerState = GetXBallController()->GetXBallPlayerState();
		if (PlayerState)
		{
			auto Textures = PlayerState->GetCustomTextures();
			for (auto it = Textures.CreateConstIterator(); it; ++it)
			{
				TargetDMI->SetTextureParameterValue(*(it->Key), it->Value);
			}
		}
		ActionList = GetXBallController()->GetTempActionBar();
		ActionList.SetNum(8,false);
		GetXBallController()->ClearTempActionBar();
	}
	SetEntireMaterial(TargetDMI);
}

void AXBallBase::Rep_Team()
{
	RefreshPlayerAppearance(Team);
}
