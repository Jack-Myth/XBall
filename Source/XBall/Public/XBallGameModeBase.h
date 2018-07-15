// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "XBallGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class XBALL_API AXBallGameModeBase : public AGameMode
{
	GENERATED_BODY()

	int32 MapSeed;
	bool bIsTeamGame=false;

	struct FPlayerRespawnInfo 
	{
		float TimeRemain;

		//This Should Be TScriptInterface<> To Hold both AIController and PlayerController
		//But Now I Doesn't Have enough time to do it, So just this.
		class AXBallPlayerControllerBase* TargetController; 
	};
	TArray<FPlayerRespawnInfo> PlayerRespawnQueue;
public:
	AXBallGameModeBase();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
		void SetIsTeamGame(bool isTeamGame);

	UFUNCTION(BlueprintPure)
		bool IsTeamGame();

	virtual void Tick(float DeltaSeconds) override;

	// Notify New Player To Build Map
	virtual void PostLogin(APlayerController* NewPlayer) override;

	UFUNCTION(BlueprintCallable)
	void PrepareReSpawn(AXBallPlayerControllerBase* TargetController);

	
	/*virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;*/


	virtual void RestartPlayerAtTransform(AController* NewPlayer, const FTransform& SpawnTransform) override;

protected:
	virtual void HandleMatchHasStarted() override;


	virtual void HandleMatchHasEnded() override;

private:
	virtual bool IsMatchInProgress() const override;

};
