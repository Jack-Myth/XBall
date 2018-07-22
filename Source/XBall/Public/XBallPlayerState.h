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

	TMap<FString, UTexture2D*> CustomTextures;
		UPROPERTY(ReplicatedUsing = "BaseTextureData_Rep")
	TArray<uint8> BaseTextureData;
		UPROPERTY(ReplicatedUsing = "NormalMapData_Rep")
	TArray<uint8> NormalMapData;
	UPROPERTY(Replicated)
		bool Lobby_IsReady = false;
public:
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation)
		void SetLobbyIsReady(bool Ready);

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

	UFUNCTION(BlueprintCallable,Server,Reliable,WithValidation)
		void SetCustomTexture(const FString& TextureParamterName,const TArray<uint8>& TextureData);
	UFUNCTION(BlueprintPure)
		inline TMap<FString, UTexture2D*> GetCustomTextures()
	{
		return CustomTextures;
	}

	UFUNCTION()
		void BaseTextureData_Rep();
	UFUNCTION()
		void NormalMapData_Rep();


	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const override;
protected:
	virtual void CopyProperties(APlayerState* PlayerState) override;

};
