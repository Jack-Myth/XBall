// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "XBallPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class XBALL_API AXBallPlayerState : public APlayerState
{
	GENERATED_BODY()
	UPROPERTY(Replicated)
		bool Lobby_IsReady = false;
public:
	AXBallPlayerState();
	UPROPERTY()
		TMap<FString, UTexture2D*> CustomTextures;
	UPROPERTY(BlueprintReadWrite)
		TArray<uint8> BaseTextureData;
	UPROPERTY()
		TArray<uint8> NormalMapData;

	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation)
		void SetLobbyIsReady(bool Ready);

	UPROPERTY(BlueprintReadOnly,ReplicatedUsing="HoldingCharacter_Rep")
		class AXBallBase* HoldingCharacter=nullptr;

	UPROPERTY()
		TArray<class FSocket*> CustomTextureSocket;
	UFUNCTION(BlueprintPure)
	inline bool IsLobbyReady()
	{
		return Lobby_IsReady;
	}
	UPROPERTY(BlueprintReadWrite,Replicated)
		int KillScore;
	UPROPERTY(BlueprintReadWrite, Replicated)
		int DeadCount;
	UPROPERTY(BlueprintReadOnly, Replicated)
		int Team = 3;
	UPROPERTY(BlueprintReadOnly, Replicated)
		bool bIsTextureReceiving = false;
	UPROPERTY()
		bool PendingSend;
	
	FTimerHandle CustomTextureTimer;

	UFUNCTION(BlueprintCallable)
		void SetCustomTexture(const FString& TextureParamterName,const TArray<uint8>& TextureData);
	UFUNCTION(BlueprintPure)
		inline TMap<FString, UTexture2D*> GetCustomTextures()
	{
		return CustomTextures;
	}

	UFUNCTION(BlueprintCallable)
		void BaseTextureData_Rep();

	DEPRECATED(4.19,"Currently NormalMap is not in the plan")
	UFUNCTION()
		void NormalMapData_Rep();

	UFUNCTION(client, Reliable)
		void NotifyGetTextureData(int Port);
	UFUNCTION()
		void SyncTextureDataToClient();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const override;

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void CopyProperties(APlayerState* PlayerState) override;

	UFUNCTION()
		void HoldingCharacter_Rep();
};
