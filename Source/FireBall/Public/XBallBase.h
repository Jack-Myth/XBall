// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ActionBase.h"
#include "ScenePlayerTarget.h"
#include "DefaultPlayerTarget.h"
#include "XBallBase.generated.h"

UCLASS()
class FIREBALL_API AXBallBase : public ACharacter
{
	GENERATED_BODY()


	class AWeaponBase* CurrentWeapon = nullptr;
	class ASkillBase* CurrentSkill = nullptr;
	//class USceneComponent* SkillSocket=nullptr, *WeaponSocket = nullptr;
	UStaticMeshComponent* CoreBallMesh=nullptr;
	class UCameraComponent* PlayerCamera = nullptr;
	int Health;
	UObject* PlayerTarget=nullptr;
	FVector GetCursorLocation(FVector* outSurfaceNormal=nullptr);
public:
	UPROPERTY(BlueprintReadWrite)
		class USpringArmComponent* CameraArm = nullptr;
	// Sets default values for this character's properties
	AXBallBase();

	UPROPERTY(BlueprintReadWrite,EditAnywhere)
		UClass* PlayerTargetClass=ADefaultPlayerTarget::StaticClass();
	UFUNCTION(BlueprintCallable)
		void UpdatePlayerTarget();
	void SetHealth(int NewHealth);
	int GetHealth();

	UFUNCTION(BlueprintCallable)
		void NotifySkillLeave();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:

	//Check If Player Should Die
	UFUNCTION(BlueprintNativeEvent)
	void CheckShouldDie();//On Server

	//Notify GameMode To Respawn Player Pawn
	UFUNCTION(BlueprintCallable)
	void NotifyRespawn();//On Server


	//On Server
	//Some Dead Logic,Default Only Spawn an Particle Emitter
	UFUNCTION(BlueprintCallable,NetMulticast,Reliable)
	void DieDefault();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Server,Reliable,WithValidation)
		void BeginSelectAction(int ActionIndex,FVector TargetLocation);

	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation)
		void EndSelectAction(int ActionIndex, FVector TargetLocation);
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation)
		void BeginFireWeapon(FVector TargetLocation);

	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation)
		void EndFireWeapon(FVector TargetLocation);

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable)
		void UpdateRotation(FVector TargetLocation);

	UFUNCTION()
		void Input_MoveForward(float AxisValue);
	UFUNCTION()
		void Input_MoveRight(float AxisValue);
	UFUNCTION()
		void Input_MouseX(float AxisValue);
	UFUNCTION()
		void Input_MouseY(float AxisValue);
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

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, WithValidation)
		void InitPlayer(int Team);



};
