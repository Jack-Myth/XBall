// Fill out your copyright notice in the Description page of Project Settings.

#include "XBallPlayerState.h"
#include "MyBPFuncLib.h"
#include "UnrealNetwork.h"
#include "XBallBase.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/PlayerController.h"
#include "Engine/NetConnection.h"
#include "IPAddress.h"
#include "Sockets.h"
#include "XBallPlayerControllerBase.h"

void AXBallPlayerState::SetCustomTexture(const FString& TextureParamterName, const TArray<uint8>& TextureData)
{
	
	if (TextureData.Num())
	{
		if (TextureParamterName=="BaseTexture")
		{
			BaseTextureData = TextureData;
		}
		else if (TextureParamterName=="NormalMap")
		{
			NormalMapData = TextureData;
		}
		CustomTextures.FindOrAdd(TextureParamterName) = UMyBPFuncLib::GetTextureFromData(TextureData);
	}
	else
	{
		CustomTextures.Remove(TextureParamterName);
	}
}

void AXBallPlayerState::SetLobbyIsReady_Implementation(bool Ready)
{
	Lobby_IsReady = Ready;
}

bool AXBallPlayerState::SetLobbyIsReady_Validate(bool Ready)
{
	return true;
}

AXBallPlayerState::AXBallPlayerState()
{
	//PrimaryActorTick.bCanEverTick = true;
}

void AXBallPlayerState::BaseTextureData_Rep()
{
	if (BaseTextureData.Num())
	{
		CustomTextures.FindOrAdd("BaseTexture") = UMyBPFuncLib::GetTextureFromData(BaseTextureData);
		if (IsValid(HoldingCharacter))
		{
			HoldingCharacter->TargetDMI->SetTextureParameterValue("BaseTexture", *(CustomTextures.Find("BaseTexture")));
		}
	}
	else
	{
		CustomTextures.FindOrAdd("BaseTexture") = nullptr;
		if (HoldingCharacter)
		{
			HoldingCharacter->TargetDMI->SetTextureParameterValue("BaseTexture", LoadObject<UTexture2D>(nullptr, TEXT("Texture2D'/Game/XBall/Materials/Textures/PureWhite.PureWhite'")));
		}
	}
}

void AXBallPlayerState::NormalMapData_Rep()
{
	if (NormalMapData.Num())
	{
		CustomTextures.FindOrAdd("NormalMap") = UMyBPFuncLib::GetTextureFromData(NormalMapData);
		if (IsValid(HoldingCharacter))
		{
			HoldingCharacter->TargetDMI->SetTextureParameterValue("NormalMap", *(CustomTextures.Find("NormalMap")));
		}
	}
	else
	{
		CustomTextures.FindOrAdd("NormalMap") = nullptr;
		if (IsValid(HoldingCharacter))
		{
			HoldingCharacter->TargetDMI->SetTextureParameterValue("NormalMap", LoadObject<UTexture2D>(nullptr, TEXT("Texture2D'/Game/XBall/Materials/Textures/FlatenNormal.FlatenNormal'")));
		}
	}
}

void AXBallPlayerState::NotifyGetTextureData_Implementation(int Port)
{
	if (UGameplayStatics::GetPlayerController(this, 0))
	{
		//Authority won't pass this test beacuse it's NetConnection will be null
		//So these code won't run on server.
		if (UGameplayStatics::GetPlayerController(this, 0)->NetConnection&&
			UGameplayStatics::GetPlayerController(this, 0)->NetConnection->URL.Host)
		{
			CustomTextureSocket.Empty();
			CustomTextureSocket.Add(FTcpSocketBuilder("CustomTextureReceiver")
				.AsNonBlocking().Build());
			if (!CustomTextureSocket[0])
			{
				CustomTextureSocket.Empty();
				return;
			}
			TSharedRef<FInternetAddr> ServerAddr = ISocketSubsystem::Get()->CreateInternetAddr();
			bool IsServerIPValid;
			ServerAddr->SetIp(*(UGameplayStatics::GetPlayerController(this, 0)->NetConnection->URL.Host), IsServerIPValid);
			if (!IsServerIPValid)
				return;
			ServerAddr->SetPort(Port);
			if (!CustomTextureSocket[0]->Connect(ServerAddr))
				return;
			BaseTextureData.Empty();
			//Async Receive.
			GetWorld()->GetTimerManager().SetTimer(CustomTextureTimer, [=]()
				{
					int PendingDataSize;
					if (CustomTextureSocket[0]->GetConnectionState() != ESocketConnectionState::SCS_Connected)
					{
						GetWorld()->GetTimerManager().ClearTimer(CustomTextureTimer);
						CustomTextureSocket[0]->Close();
						CustomTextureSocket.Empty();
					}
					if (CustomTextureSocket[0]->HasPendingData(PendingDataSize))
					{
						TArray<uint8> tmpData;
						tmpData.SetNum(PendingDataSize, false);
						int ByteRead;
						CustomTextureSocket->Recv(tmpData.GetData(), PendingDataSize, ByteRead);
						BaseTextureData.Append(tmpData);
					}
				}, 0.02f, true);
		}
	}
}

void AXBallPlayerState::SyncTextureDataToClient()
{
	CustomTextureSocket.Add(FTcpSocketBuilder::AsNonBlocking().AsReusable().Build());
	GetWorld()->GetTimerManager().SetTimer(CustomTextureTimer, [=]()
		{
			bool HasPendingConnection;
			CustomTextureSocket[0]->HasPendingConnection(HasPendingConnection);
			while (HasPendingConnection)
			{
				FSocket* tempSocket = CustomTextureSocket[0]->Accept("PlayerStateClientSocket");
				int ByteSent;
				tempSocket->Send(BaseTextureData.GetData(), BaseTextureData.Num(),ByteSent);
				if (ByteSent!=BaseTextureData.Num())
				{
					UE_LOG(LogTemp, Error, TEXT("Server-Client Custom Texture Sync Failed:ByteSent is not equal to Data Count"));
				}
				tempSocket->Close();
				CustomTextureSocket.Add(tempSocket);
				CustomTextureSocket[0]->HasPendingConnection(HasPendingConnection);
			}
			TArray<AActor*> Controllers;
			UGameplayStatics::GetAllActorsOfClass(this, AXBallPlayerControllerBase::StaticClass(), Controllers);
			if (CustomTextureSocket.Num()>=Controllers.Num())
			{
				CustomTextureSocket[0]->Close();
				CustomTextureSocket.Empty();
				GetWorld()->GetTimerManager().ClearTimer(CustomTextureTimer);
			}
		}, 0.02f, true);
}

void AXBallPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AXBallPlayerState, KillScore);
	DOREPLIFETIME(AXBallPlayerState, DeadCount);
	DOREPLIFETIME(AXBallPlayerState, Team);
	DOREPLIFETIME(AXBallPlayerState, Lobby_IsReady);
	//DOREPLIFETIME(AXBallPlayerState, NormalMapData);
	//DOREPLIFETIME(AXBallPlayerState, BaseTextureData);
	DOREPLIFETIME(AXBallPlayerState, HoldingCharacter);
}

void AXBallPlayerState::Tick(float DeltaSeconds)
{
	if (!IsValid(HoldingCharacter))
		return;
	if (CustomTextures.Find("BaseTexture") && *CustomTextures.Find("BaseTexture"))
		HoldingCharacter->TargetDMI->SetTextureParameterValue("BaseTexture", *(CustomTextures.Find("BaseTexture")));
	else
		HoldingCharacter->TargetDMI->SetTextureParameterValue("BaseTexture", LoadObject<UTexture2D>(nullptr, TEXT("Texture2D'/Game/XBall/Materials/Textures/PureWhite.PureWhite'")));
	if (CustomTextures.Find("NormalMap") && *CustomTextures.Find("NormalMap"))
		HoldingCharacter->TargetDMI->SetTextureParameterValue("NormalMap", *(CustomTextures.Find("NormalMap")));
	else
		HoldingCharacter->TargetDMI->SetTextureParameterValue("NormalMap", LoadObject<UTexture2D>(nullptr, TEXT("Texture2D'/Game/XBall/Materials/Textures/FlatenNormal.FlatenNormal'")));
	HoldingCharacter->SetEntireMaterial(HoldingCharacter->TargetDMI);
}

void AXBallPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);
	AXBallPlayerState* XBallPlayerState = Cast<AXBallPlayerState>(PlayerState);
	if (XBallPlayerState)
	{
		//KillScore And DeadCount may use for out-game rank
		XBallPlayerState->KillScore = KillScore;
		XBallPlayerState->DeadCount = DeadCount;
		XBallPlayerState->NormalMapData = NormalMapData;
		XBallPlayerState->BaseTextureData = BaseTextureData;
		XBallPlayerState->Team = Team;
		XBallPlayerState->CustomTextures.FindOrAdd("BaseTexture") = UMyBPFuncLib::GetTextureFromData(BaseTextureData);
		XBallPlayerState->CustomTextures.FindOrAdd("NormalMap") = UMyBPFuncLib::GetTextureFromData(NormalMapData);
	}
}

void AXBallPlayerState::HoldingCharacter_Rep()
{
	if (!IsValid(HoldingCharacter))
		return;
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, HoldingCharacter->GetName());
	if (CustomTextures.Find("BaseTexture")&&*CustomTextures.Find("BaseTexture"))
		HoldingCharacter->TargetDMI->SetTextureParameterValue("BaseTexture", *(CustomTextures.Find("BaseTexture")));
	else
		HoldingCharacter->TargetDMI->SetTextureParameterValue("BaseTexture", LoadObject<UTexture2D>(nullptr, TEXT("Texture2D'/Game/XBall/Materials/Textures/PureWhite.PureWhite'")));
	if (CustomTextures.Find("NormalMap")&& *CustomTextures.Find("NormalMap"))
		HoldingCharacter->TargetDMI->SetTextureParameterValue("NormalMap", *(CustomTextures.Find("NormalMap")));
	else
		HoldingCharacter->TargetDMI->SetTextureParameterValue("NormalMap", LoadObject<UTexture2D>(nullptr, TEXT("Texture2D'/Game/XBall/Materials/Textures/FlatenNormal.FlatenNormal'")));
	HoldingCharacter->SetEntireMaterial(HoldingCharacter->TargetDMI);
}
