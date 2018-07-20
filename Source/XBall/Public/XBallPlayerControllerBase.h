// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "IMapGenerator.h"
#include "ActionBase.h"
#include "XBallBase.h"
#include "XBallPlayerState.h"
#include "XBallPlayerControllerBase.generated.h"

UENUM(BlueprintType)
enum class EScoreType: uint8
{
	ST_Kill		UMETA(DisplayName="Kill"),
	ST_Dead		UMETA(DisplayName = "Dead"),
	ST_ALL		UMETA(Hidden)
};

/**
 * 
 */
UCLASS()
class XBALL_API AXBallPlayerControllerBase : public APlayerController
{
	GENERATED_BODY()

	UUserWidget* ActionInventoryWidget = nullptr;
	UUserWidget* MainUIWidget = nullptr;
	UUserWidget* RankWidget = nullptr;
	UUserWidget* WaitRespawn = nullptr;

	UPROPERTY(Replicated)
		bool bIsInLobby=true;
protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		UClass* PlayerDefaultCharacter;
	UPROPERTY(Replicated)
		TArray<AActionBase*> ActionInventory;
	UPROPERTY(Replicated)
		TArray<AActionBase*> TempActionBar;

	UPROPERTY(BlueprintReadWrite,Replicated)
		int Coins=800;

	virtual void SetupInputComponent() override;

public:
	AXBallPlayerControllerBase();

	UFUNCTION(BlueprintCallable, NetMulticast,Reliable)
		void SetTeam(int NewTeam);
	UFUNCTION(BlueprintCallable)
		int GetTeam();
	UFUNCTION(BlueprintCallable,NetMulticast,Reliable,WithValidation)
		void SetPlayerDefaultCharacter(TSubclassOf<AXBallBase> DefaultCharacter);
	
	inline TArray<AActionBase*> GetTempActionBar()
	{
		return TempActionBar;
	}

	inline void ClearTempActionBar()
	{
		TempActionBar.Empty(8);
	}

	inline UClass * GetPlayerDefaultCharacter()
	{
		return PlayerDefaultCharacter;
	}

	UFUNCTION(BlueprintPure)
		inline TArray<AActionBase*> GetActionBarItems()
	{
		AXBallBase* XballBase = Cast<AXBallBase>(GetCharacter());
		return XballBase ? XballBase->GetActionBarItems() : TempActionBar;
	}

	UFUNCTION(BlueprintPure)
		class UUserWidget* FindActionBarItemWidgetFor(AActionBase* ActionInstance);

	/*UFUNCTION(BlueprintCallable,Client, Reliable, WithValidation)
		void BuildMap(UClass* MapGenerator, int MaxEngth, int MaxWidth, int MaxHeight,int32 NoiseSeed);*/
	UFUNCTION(BlueprintCallable,Client, Reliable, WithValidation)
		void ReGenOldMap(UClass* MapGenerator,int MaxEngth, int MaxWidth, int MaxHeight, int32 Seed,const TArray<FBlockInfo>& BlockInfo);
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
		void SetIsInLobby(bool mIsInLobby)
	{
		bIsInLobby = mIsInLobby;
	}

	UFUNCTION(BlueprintPure)
		inline bool IsInLobby()
	{
		return bIsInLobby;
	}

	//virtual void Possess(APawn* aPawn) override;

	void ModifyScore(EScoreType Type);
	void ClearScore(EScoreType Type);
	int GetScore(EScoreType Type);

	UFUNCTION(BlueprintCallable, Server, Reliable,WithValidation)
		void UpdatePicData(const FString& TextureParamName,const TArray<uint8>& TextureData);

	UFUNCTION(BlueprintPure)
	inline class AXBallPlayerState* GetXBallPlayerState()
	{
		return Cast<AXBallPlayerState>(PlayerState);
	}

	UFUNCTION(BlueprintPure)
		inline TArray<AActionBase*> GetActionInventory()
	{
		return ActionInventory;
	}

	bool Buy_Internal(TSubclassOf<AActionBase> ActionClass,FString& RetMessage);

	//Move Action from Action Inventory To ActionBar
	UFUNCTION(BlueprintCallable,Server,Reliable,WithValidation)
		void MoveActionToActionBar(AActionBase* ActionInstance,int AddToIndex);

	//Move Action from ActionBar to Action Inventory
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation)
		void MoveActionToInventory(int ActionBarIndex);

	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation)
		void SwapActionBarItem(int IndexA,int IndexB);

	UFUNCTION(BlueprintCallable, Server, Reliable,WithValidation)
		void Buy(TSubclassOf<AActionBase> ActionClass);

	UFUNCTION(BlueprintCallable, Client, Reliable)
		void ShowMessage(const FString& Title, const FString& Message);

	UFUNCTION(BlueprintCallable, Client, Reliable)
		void ShowWarning(const FString& Title, const FString& Message);

	UFUNCTION(BlueprintCallable, Client, Reliable)
		void ShowOK(const FString& Title, const FString& Message);

	UFUNCTION()
		void ToggleActionInventory();

	UFUNCTION()
		void OpenRank();
	UFUNCTION()
		void CloseRank();

	UFUNCTION(BlueprintCallable,Client,Reliable)
		void InitGameUI();

	UFUNCTION(BlueprintCallable, Client, Reliable)
		void ShowWaitingRespawn();
	UFUNCTION(BlueprintCallable, Client, Reliable)
		void CloseWaitingRespawn();


	//Show Game Result
	//If WinTeam is less than 0(-1),It means the game isn't Team Play Game.
	UFUNCTION(BlueprintCallable,Client,Reliable)
		void ShowResultSync(int WinTeam);
};
