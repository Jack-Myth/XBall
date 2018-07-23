// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ActionBase.h"
#include "ScenePlayerTarget.h"
#include "DefaultPlayerTarget.h"
#include "XBallBase.generated.h"

UCLASS()
class XBALL_API AXBallBase : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(Replicated)
		class AWeaponBase* CurrentWeapon = nullptr;
	UPROPERTY(Replicated)
		class ASkillBase* CurrentSkill = nullptr;
	//class USceneComponent* SkillSocket=nullptr, *WeaponSocket = nullptr;
	UStaticMeshComponent* CoreBallMesh=nullptr;
	class UCameraComponent* PlayerCamera = nullptr;
	UPROPERTY(Replicated)
		int Health=100;
	UObject* PlayerTarget=nullptr;
	FVector GetCursorLocation(FVector* outSurfaceNormal=nullptr);
	FVector TargetLocationCache;
	FVector MovementDir;
	FTimerHandle ScreenEffectDamage_TimerHandle;

	UPROPERTY(Replicated)
		TArray<AActionBase*> ActionList;

	UFUNCTION(Server, Reliable, WithValidation)
		void Sprint(FVector Dir);

	float SprintCoolDown;
	FTimerHandle SprintCoolDownTimerHandle;

	UPROPERTY(Replicated)
		bool bSprintable=true;
public:
	UPROPERTY(BlueprintReadWrite)
		UMaterialInstanceDynamic * TargetDMI;

	UFUNCTION(BlueprintPure)
	inline bool IsSprintable()
	{
		return bSprintable;
	}

	UFUNCTION(BlueprintPure, BlueprintNativeEvent)
		FString GetXBallName();

	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation)
		void DoReload();

	UPROPERTY(BlueprintReadWrite,EditAnywhere)
		class USpringArmComponent* CameraArm = nullptr;
	// Sets default values for this character's properties
	AXBallBase();

	AActionBase* AddActionToBar(int Index, AActionBase* Action);
	
	AActionBase* RemoveActionFromBar(int Index);
	
	inline TArray<AActionBase*> GetActionBarItems()
	{
		return ActionList;
	}

	UPROPERTY(BlueprintReadWrite,EditAnywhere)
		UClass* PlayerTargetClass=ADefaultPlayerTarget::StaticClass();
	UFUNCTION(BlueprintCallable)
		void UpdatePlayerTarget();

	UFUNCTION(BlueprintPure)
	inline AWeaponBase* GetCurrentWeapon()
	{
		return CurrentWeapon;
	}

	UFUNCTION(BlueprintPure)
	inline ASkillBase* GetCurrentSkill()
	{
		return CurrentSkill;
	}

	// It's Not a RPC Function, But don't call It on Client.
	UFUNCTION(BlueprintCallable)
	void SetHealth(int NewHealth)
	{
		Health = NewHealth;
		CheckShouldDie(nullptr);
	}

	UFUNCTION(BlueprintPure)
	inline int GetHealth()
	{
		return Health;
	}

		void NotifySkillLeave();

	UPROPERTY(ReplicatedUsing="Rep_Team")
		int Team=-1;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


	virtual void PossessedBy(AController* NewController) override;


	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:

	UMaterialInstanceDynamic * DamagedEffect;
	float DamageBlendAlpha = 0.f;

	//Check If Player Should Die
	//InstigatedBy can be nullptr, It means no one cause Damage.
	UFUNCTION(BlueprintNativeEvent)
	void CheckShouldDie(AController* InstigatedBy=nullptr);//On Server

	//Notify GameMode To Respawn Player Pawn
	UFUNCTION(BlueprintCallable)
	void NotifyRespawn();//On Server


	//On Server
	//Some Dead Logic,Default Only Spawn an Particle Emitter
	//InstigatedBy can be nullptr, It means no one will get score when this Player died.
	UFUNCTION(BlueprintCallable,NetMulticast,Reliable)
	void DieDefault(AController* InstigatedBy=nullptr);

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable,Client,Unreliable)
		void ShowScreenEffectDamaged_Rep(int Damage, const AController* Instigater, const class AActor* DamageCauser, const class UDamageType* DamageType);

	// Show Screen Effect When Take Damage
	// May be overrided By child Class
	// Called By ShowScreenEffectDamaged_Rep
	UFUNCTION(BlueprintNativeEvent)
		void ShowScreenEffectDamaged(int Damage, const AController* Instigater, const class AActor* DamageCauser, const class UDamageType* DamageType);

	// Call while Press an Action
	UFUNCTION(BlueprintCallable, Server,Reliable,WithValidation)
		void BeginSelectAction(int ActionIndex,FVector TargetLocation);

	// Call while release an Action
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation)
		void EndSelectAction(int ActionIndex, FVector TargetLocation);

	// Call while Press Fire Button
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation)
		void BeginFireWeapon(FVector TargetLocation);

	// Call while release fire button
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation)
		void EndFireWeapon(FVector TargetLocation);

	// Update Player Character On Server.
	UFUNCTION(BlueprintCallable, Server, Unreliable,WithValidation)
		void UpdateRotation(FVector TargetLocation);

	// Update Rotation Locally
	// Called By UpdateRotation while Execute on server.
	// Client May also call this function to update rotation locally.
	void UpdateRotation_Internal(FVector TargetLocation);
	
	// Default Processor while take any damage.
	// It will decrease the health and call CheckSouldDie()
	// then It will call ShowScreenEffectDamage() to show screen effect.
	UFUNCTION()
		void AnyDamage_Internal(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	//UFUNCTION()

	// Input Handler================================================
	UFUNCTION()
		void Input_MoveForward(float AxisValue);
	UFUNCTION()
		void Input_MoveRight(float AxisValue);
	UFUNCTION()
		void Input_MouseX(float AxisValue);
	UFUNCTION()
		void Input_MouseY(float AxisValue);
	UFUNCTION()
		void Input_Reload();
	UFUNCTION()
		void Input_Sprint();
	UFUNCTION()
		void Input_JumpStart();
	UFUNCTION()
		void Input_JumpEnd();
	UFUNCTION()
		void Input_BeginFire();
	UFUNCTION()
		void Input_EndFire();
	UFUNCTION()
		void Input_BeginAction1();
	UFUNCTION()
		void Input_EndAction1();
	UFUNCTION()
		void Input_BeginMainAction();
	UFUNCTION()
		void Input_EndMainAction();
	UFUNCTION()
		void Input_BeginAction2();
	UFUNCTION()
		void Input_EndAction2();
	UFUNCTION()
		void Input_BeginAction3();
	UFUNCTION()
		void Input_EndAction3();
	UFUNCTION()
		void Input_BeginAction4();
	UFUNCTION()
		void Input_EndAction4();
	UFUNCTION()
		void Input_BeginAction5();
	UFUNCTION()
		void Input_EndAction5();
	UFUNCTION()
		void Input_BeginAction6();
	UFUNCTION()
		void Input_EndAction6();
	UFUNCTION()
		void Input_BeginAction7();
	UFUNCTION()
		void Input_EndAction7();

	//End Input Handler==================================

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Init Team and Player Material
	UFUNCTION(BlueprintCallable)
		void RefreshPlayerAppearance(int mTeam);

	UFUNCTION()
		void Rep_Team();

	UFUNCTION(BlueprintCallable)
		class AXBallPlayerControllerBase* GetXBallController();

	UFUNCTION(BlueprintNativeEvent,BlueprintCallable)
		void SetEntireMaterial(UMaterialInterface* Mat);
};
