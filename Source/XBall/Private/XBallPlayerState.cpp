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
#include "SocketSubsystem.h"
#include "Engine/World.h"
#include "Runtime/Networking/Public/Common/TcpSocketBuilder.h"
#include "XBallGameModeBase.h"

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

void AXBallPlayerState::GenBaseTexture()
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

void AXBallPlayerState::NotifyGetTextureData_Implementation(int Port,int DataSize)
{
	if (UGameplayStatics::GetPlayerController(this, 0))
	{
		//Authority won't pass this test beacuse it's NetConnection will be null
		//So these code won't run on server.
		if (UGameplayStatics::GetPlayerController(this, 0)->NetConnection&&
			UGameplayStatics::GetPlayerController(this, 0)->NetConnection->URL.Host != "")
		{
			CustomTextureSocket=MakeShareable(FTcpSocketBuilder("CustomTextureReceiver").AsNonBlocking().Build());
			if (!CustomTextureSocket.IsValid())
			{
				return;
			}
			TSharedRef<FInternetAddr> ServerAddr = ISocketSubsystem::Get()->CreateInternetAddr();
			bool IsServerIPValid;
			ServerAddr->SetIp(*(UGameplayStatics::GetPlayerController(this, 0)->NetConnection->URL.Host), IsServerIPValid);
			if (!IsServerIPValid)
				return;
			ServerAddr->SetPort(Port);
			if (!CustomTextureSocket->Connect(ServerAddr.Get()))
				return;
			BaseTextureData.Empty();
			//Async Receive.
			GetWorld()->GetTimerManager().SetTimer(CustomTextureTimer, [=]()
				{
					uint32 PendingDataSize;
					if (CustomTextureSocket->HasPendingData(PendingDataSize))
					{
						TArray<uint8> tmpData;
						tmpData.SetNum(PendingDataSize, false);
						int ByteRead;
						CustomTextureSocket->Recv(tmpData.GetData(), PendingDataSize, ByteRead);
						BaseTextureData.Append(tmpData);
					}
					if (CustomTextureSocket->GetConnectionState() != ESocketConnectionState::SCS_Connected||BaseTextureData.Num()>=DataSize)
					{
						GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Green, "Client Disconnet");
						//Disconnect and Gen Texture.
						GetWorld()->GetTimerManager().ClearTimer(CustomTextureTimer);
						CustomTextureSocket->Close();
						GenBaseTexture();
					}
				}, 0.02f, true);
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Green, "Invalid NetConnection");
		}
	}
	else
		GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, "Invalid PlayerController");
}

void AXBallPlayerState::SyncTextureDataToClient()
{
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Cyan, "Begin Sync TextureData To Client");
	CustomTextureSocket=MakeShareable(FTcpSocketBuilder("SyncTextureServerSocket").AsNonBlocking().AsReusable().Build());
	TSharedRef<FInternetAddr> LocalAddr = ISocketSubsystem::Get()->GetLocalBindAddr(*GLog);
	int TargetPort = ISocketSubsystem::Get()->BindNextPort(CustomTextureSocket.Get(), LocalAddr.Get(), 5, 1);
	if (!TargetPort)
		return;
	CustomTextureSocket->Listen(32);
	bIsTextureSending = true;
	GetWorld()->GetTimerManager().SetTimer(CustomTextureTimer, [=]()
		{
			bool HasPendingConnection;
			CustomTextureSocket->HasPendingConnection(HasPendingConnection);
			while (HasPendingConnection)
			{
				FSocket* tempSocket = CustomTextureSocket->Accept("PlayerStateClientSocket");
				int ByteSent;
				tempSocket->Send(BaseTextureData.GetData(), BaseTextureData.Num(),ByteSent);
				tempSocket->Close();
				CustomTextureSocket->HasPendingConnection(HasPendingConnection);
			}
			//Socket will always open for new player.
			/*if (CustomTextureSocket.Num()>=Controllers.Num()&&IsAllSocketClosed)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, "Socket Closed");
				CustomTextureSocket[0]->Close();
				CustomTextureSocket.Empty();
				GetWorld()->GetTimerManager().ClearTimer(CustomTextureTimer);
			}*/
		}, 0.02f, true);
	CustomTexturePort = TargetPort;
	CustomTextureSize = BaseTextureData.Num();
	//NotifyGetTextureData(TargetPort,BaseTextureData.Num());
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
	DOREPLIFETIME(AXBallPlayerState, CustomTexturePort);
	DOREPLIFETIME(AXBallPlayerState, CustomTextureSize);
	DOREPLIFETIME(AXBallPlayerState, IsHost);
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

void AXBallPlayerState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CustomTextureSocket.IsValid())
	{
		CustomTextureSocket->Close();
		GetWorld()->GetTimerManager().ClearTimer(CustomTextureTimer);
	}
}

void AXBallPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);
	AXBallPlayerState* XBallPlayerState = Cast<AXBallPlayerState>(PlayerState);
	if (XBallPlayerState)
	{
		//XBallPlayerState->KillScore = KillScore;
		//XBallPlayerState->DeadCount = DeadCount;
		XBallPlayerState->NormalMapData = NormalMapData;
		XBallPlayerState->BaseTextureData = BaseTextureData;
		XBallPlayerState->Team = Team;
		if (BaseTextureData.Num()&&Cast<AXBallGameModeBase>(UGameplayStatics::GetGameMode(this)))
		{
			XBallPlayerState->SyncTextureDataToClient();
		}
		XBallPlayerState->CustomTextures.FindOrAdd("BaseTexture") = UMyBPFuncLib::GetTextureFromData(BaseTextureData);
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

void AXBallPlayerState::CustomTexturePort_Rep()
{
	if (CustomTextureSize!=0)
	{
		NotifyGetTextureData(CustomTexturePort, CustomTextureSize);
	}
}

void AXBallPlayerState::CustomTextureSize_Rep()
{
	if (CustomTextureSize != 0&& CustomTexturePort!=0)
	{
		NotifyGetTextureData(CustomTexturePort, CustomTextureSize);
	}
}
