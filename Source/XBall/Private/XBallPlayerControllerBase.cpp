// Fill out your copyright notice in the Description page of Project Settings.

#include "XBallPlayerControllerBase.h"
#include "MyBPFuncLib.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "UnrealNetwork.h"
#include "XBallPlayerState.h"
#include "UserWidget.h"
#include "WidgetBlueprintLibrary.h"
#include "XBallGameModeBase.h"
#include "SocketSubsystem.h"
#include "Sockets.h"
#include "Runtime/Networking/Public/Common/TcpSocketBuilder.h"
#include "IPAddress.h"
#include "LobbyGameModeBase.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialParameterCollection.h"

void AXBallPlayerControllerBase::ReGenOldMap_Implementation(UClass* MapGenerator,int MaxEngth, int MaxWidth, int MaxHeight, int32 Seed,const TArray<FBlockInfo>& BlockInfo)
{
	GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Blue, "Controller Executed RPC");
	if (this->HasAuthority())
	{
		GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, "Warning:Server Is Executeing the ReGenOldMap()!");
	}
	else
		GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Green, "OK:Client Is Executeing the ReGenOldMap().");
	UMyBPFuncLib::ClearMap(this);
	UMyBPFuncLib::GenWorld(this, MaxEngth, MaxWidth, MaxHeight, Seed);
	UMyBPFuncLib::SyncMap(this,BlockInfo);
}

bool AXBallPlayerControllerBase::ReGenOldMap_Validate(UClass* MapGenerator,int MaxEngth, int MaxWidth, int MaxHeight, int32 Seed, const TArray<FBlockInfo>& BlockInfo)
{
	return true;
}

void AXBallPlayerControllerBase::SetupInputComponent()
{
	Super::SetupInputComponent();
	InputComponent->BindAction("Inventory", EInputEvent::IE_Pressed, this, &AXBallPlayerControllerBase::ToggleActionInventory);
	InputComponent->BindAction("Rank", EInputEvent::IE_Pressed, this, &AXBallPlayerControllerBase::OpenRank);
	InputComponent->BindAction("Rank", EInputEvent::IE_Released, this, &AXBallPlayerControllerBase::CloseRank);
	InputComponent->BindAction("EscapeMenu", EInputEvent::IE_Pressed, this, &AXBallPlayerControllerBase::OpenEscMenu);
	InputComponent->BindAction("Chat", EInputEvent::IE_Pressed, this, &AXBallPlayerControllerBase::OpenChatUI);
}

void AXBallPlayerControllerBase::BeginPlay()
{
	if (HasAuthority()&&IsLocalController())
	{
		if (GetXBallPlayerState())
		{
			GetXBallPlayerState()->IsHost = true;
		}
	}
	if (IsLocalController()&&!IsInLobby())
	{
		TSubclassOf<UUserWidget> ChatUIClass = LoadClass<UUserWidget>(nullptr, TEXT("WidgetBlueprint'/Game/XBall/Blueprints/UMG/ChatUI.ChatUI_C'"));
		if (ChatUIClass)
		{
			ChatUIWidget = UUserWidget::CreateWidgetOfClass(ChatUIClass, nullptr, nullptr, this);
			if (ChatUIWidget)
			{
				ChatUIWidget->AddToViewport();
			}
		}
		MatData = LoadObject<UMaterialParameterCollection>(nullptr, TEXT("MaterialParameterCollection'/Game/XBall/Materials/MatData.MatData'"));
	}
}

void AXBallPlayerControllerBase::NotifySendCustomTexture_Implementation(int Port)
{
	CustomTextureSockets.Empty();
	CustomTextureSockets.Add(MakeShareable(FTcpSocketBuilder("CustomTextureClientSender").AsNonBlocking().AsReusable().Build()));
	if (!CustomTextureSockets[0].IsValid())
		return;
	if (NetConnection&&NetConnection->URL.Host != "")
	{
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, NetConnection->URL.Host);
		TSharedRef<FInternetAddr> ServerAddr= ISocketSubsystem::Get()->CreateInternetAddr();
		bool IsServerHostValid;
		ServerAddr->SetIp(*(NetConnection->URL.Host),IsServerHostValid);
		if (!IsServerHostValid)
			return;
		ServerAddr->SetPort(Port);
		if (CustomTextureSockets[0]->Connect(ServerAddr.Get())&&GetXBallPlayerState())
		{
			int ByteSent;
			CustomTextureSockets[0]->Send(GetXBallPlayerState()->BaseTextureData.GetData(), GetXBallPlayerState()->BaseTextureData.Num(), ByteSent);
		}
		CustomTextureSockets[0]->Close();
		CustomTextureSockets.Empty();
	}
}

AXBallPlayerControllerBase::AXBallPlayerControllerBase()
{
	TempActionBar.SetNum(8,false);
}

bool AXBallPlayerControllerBase::SetTeam_Validate(int NewTeam)
{
	return true;
}

int AXBallPlayerControllerBase::GetTeam()
{
	AXBallPlayerState* tmpPlayerState = GetXBallPlayerState();
	if (tmpPlayerState)
	{
		return tmpPlayerState->Team;
	}
	return 0;
}

void AXBallPlayerControllerBase::SetTeam_Implementation(int NewTeam)
{
	ClearScore(EScoreType::ST_Kill);
	AXBallPlayerState* tmpPlayerState = GetXBallPlayerState();
	if (tmpPlayerState)
	{
		if (IsInLobby())
		{
			if (Cast<ALobbyGameModeBase>(UGameplayStatics::GetGameMode(this))->bIsTeamPlay)
			{
				if (NewTeam>0&&NewTeam<5)
					tmpPlayerState->Team = NewTeam;
				else
					tmpPlayerState->Team = 1;
			}
			else
			{
				tmpPlayerState->Team = 0;
			}
		}
		else
		{
			if (Cast<AXBallGameModeBase>(UGameplayStatics::GetGameMode(this))->IsTeamGame())
			{
				if (NewTeam > 0 && NewTeam < 5)
					tmpPlayerState->Team = NewTeam;
				else
					tmpPlayerState->Team = 1;
			}
			else
			{
				tmpPlayerState->Team = 0;
			}
		}
	}
}

class UUserWidget* AXBallPlayerControllerBase::FindActionBarItemWidgetFor(AActionBase* ActionInstance)
{
	if (MainUIWidget)
	{
		int index;
		AXBallBase* XballCharacter = Cast<AXBallBase>(GetCharacter());
		if (XballCharacter)
		{
			index = XballCharacter->GetActionBarItems().Find(ActionInstance);
		}
		else
		{
			index = TempActionBar.Find(ActionInstance);
		}
		if (index != INDEX_NONE)
		{
			// Use Unreal Reflection System to Call Blueprint Function And Get Result.
			UFunction* FAIW_Function = MainUIWidget->FindFunction("FindActionItemWidget");
			uint8* Buffer= (uint8*)FMemory::Malloc(FAIW_Function->ParmsSize);
			*(int*)Buffer = index;
			MainUIWidget->ProcessEvent(FAIW_Function,Buffer);
			UProperty* ReturnProperty=nullptr;
			for (TFieldIterator<UProperty> PropIt(FAIW_Function, EFieldIteratorFlags::ExcludeSuper); PropIt; ++PropIt)
			{
				if ((*PropIt)->HasAnyPropertyFlags(CPF_OutParm))
				{
					ReturnProperty = *PropIt;
					break;
				}
			}
			if (ReturnProperty)
			{
				UUserWidget* TargetWidget = (UUserWidget*)(*(ReturnProperty->ContainerPtrToValuePtr<UUserWidget*>(Buffer)));
				FMemory::Free(Buffer);
				return TargetWidget;
			}
			else
			{
				FMemory::Free(Buffer);
			}
		}
	}
	return nullptr;
}

void AXBallPlayerControllerBase::PostSeamlessTravel()
{
	AXBallGameModeBase* XBallGameMode = Cast<AXBallGameModeBase>(UGameplayStatics::GetGameMode(this));
	//Local controller shouldn't be Init Directly.
	//It will be Init by PostSeamlessTravel() In XBallGameModeBase.
	if (XBallGameMode&&!IsLocalController())
	{
		//Init PlayerController
		//GenMap, Get default value etc..
		XBallGameMode->InitPlayerController(this);
	}
}

void AXBallPlayerControllerBase::PreClientTravel(const FString& PendingURL, ETravelType TravelType, bool bIsSeamlessTravel)
{
	ActionInventoryWidget = nullptr;
	MainUIWidget = nullptr;
	RankWidget = nullptr;
	WaitRespawn = nullptr;
	EscMenuWidget = nullptr;
	ChatUIWidget = nullptr;
	Super::PreClientTravel(PendingURL, TravelType, bIsSeamlessTravel);
	if (bIsSeamlessTravel)
	{
		//Remove All widget from Viewport
		//Just the same as OpenLevel,Construct Widget by Next Level
		TArray<UUserWidget*> Widgets;
		UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this, Widgets, UUserWidget::StaticClass(), false);
		for (UUserWidget*& Widget : Widgets)
		{
			Widget->RemoveFromViewport();
		}
	}
}

void AXBallPlayerControllerBase::SeamlessTravelTo(class APlayerController* NewPC)
{
	Super::SeamlessTravelTo(NewPC);
	AXBallPlayerControllerBase* XBallPlayerController = Cast<AXBallPlayerControllerBase>(NewPC);
	XBallPlayerController->PlayerDefaultCharacter = PlayerDefaultCharacter;
}

void AXBallPlayerControllerBase::SendCustomTexture_Implementation(int DataSize)
{
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, "Begin Send Custom Texture");
	CustomTextureSockets.Empty();
	CustomTextureSockets.Add(MakeShareable(FTcpSocketBuilder("SendCustomTexture").AsNonBlocking().AsReusable().Build()));
	if (!CustomTextureSockets[0].IsValid())
		return;
	TSharedRef<FInternetAddr> LocalAddr= ISocketSubsystem::Get()->GetLocalBindAddr(*GLog);
	int TargetPort = ISocketSubsystem::Get()->BindNextPort(CustomTextureSockets[0].Get(), LocalAddr.Get(), 5, 1);
	if (!TargetPort)
		return;
	CustomTextureSockets[0]->Listen(5);
	GetWorld()->GetTimerManager().SetTimer(CustomTextureTimerHandle, [=]()
		{
			bool HasPendingConnection;
			if (CustomTextureSockets.Num() <= 1)
			{
				CustomTextureSockets[0]->HasPendingConnection(HasPendingConnection);
				if (HasPendingConnection)
				{
					CustomTextureSockets.Add(MakeShareable(CustomTextureSockets[0]->Accept("CustomTextureServerReceiver")));
				}
				else
				{
					return;
				}
			}
			uint32 PendingDataSize;
			if (CustomTextureSockets[1]->HasPendingData(PendingDataSize))
			{
				TArray<uint8> tmpData;
				tmpData.SetNum(PendingDataSize, false);
				int ByteRead;
				CustomTextureSockets[1]->Recv(tmpData.GetData(), PendingDataSize, ByteRead);
				if (GetXBallPlayerState())
				{
					GetXBallPlayerState()->BaseTextureData.Append(tmpData);
				}
			}
			if (CustomTextureSockets[1]->GetConnectionState()!=ESocketConnectionState::SCS_Connected||(GetXBallPlayerState()&& GetXBallPlayerState()->BaseTextureData.Num()>=DataSize))
			{
				//Disconnect And Sync To Client.
				CustomTextureSockets[0]->Close();
				CustomTextureSockets[1]->Close();
				CustomTextureSockets.Empty();
				GetWorld()->GetTimerManager().ClearTimer(CustomTextureTimerHandle);
				//Gen Texture.
				if (GetXBallPlayerState()&&!IsInLobby())
				{
					GetXBallPlayerState()->GenBaseTexture();
					GetXBallPlayerState()->SyncTextureDataToClient();
				}
			}
		}, 0.02f, true);
	NotifySendCustomTexture(TargetPort);
}

bool AXBallPlayerControllerBase::SendCustomTexture_Validate(int DataSize)
{
	if (GetXBallPlayerState()&&DataSize<128*1024)
	{
		if (GetXBallPlayerState()->bIsTextureSending)
		{
			return false;
		}
		else
			return true;
	}
	else
		return false;
}

void AXBallPlayerControllerBase::Tick(float DeltaSeconds)
{
	if (IsLocalController())
	{
		if (GetPawn())
		{
			UKismetMaterialLibrary::SetVectorParameterValue(this, MatData, "PlayerLocation", FLinearColor(GetPawn()->GetActorLocation()));
		}
	}
}

void AXBallPlayerControllerBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AXBallPlayerControllerBase, ActionInventory)
	DOREPLIFETIME(AXBallPlayerControllerBase, TempActionBar);
	DOREPLIFETIME(AXBallPlayerControllerBase, bIsInLobby);
	DOREPLIFETIME(AXBallPlayerControllerBase, Coins);
	DOREPLIFETIME(AXBallPlayerControllerBase, PlayerDefaultCharacter);
	DOREPLIFETIME(AXBallPlayerControllerBase, bIsTeamPlay);
}

void AXBallPlayerControllerBase::SetOnReceiveChatMessageDelegate(FOnReceiveChatMessage TargetDelegate)
{
	OnReceiveChatMsgDelegate = TargetDelegate;
}

bool AXBallPlayerControllerBase::SendChatMessage_Validate(const FString& ChatMsg)
{
	if (ChatMsg.Len()>10240)
	{
		return false;
	}
	return true;
}

void AXBallPlayerControllerBase::SendChatMessage_Implementation(const FString& ChatMsg)
{
	TArray<AActor*> XBallControllers;
	UGameplayStatics::GetAllActorsOfClass(this, AXBallPlayerControllerBase::StaticClass(), XBallControllers);
	for (AActor*& XBallController:XBallControllers)
	{
		((AXBallPlayerControllerBase*)XBallController)->DispartchChatMessage(PlayerState->GetPlayerName()+":"+ChatMsg);
	}
}

void AXBallPlayerControllerBase::DispartchChatMessage_Implementation(const FString& ChatMsg)
{
	OnReceiveChatMsgDelegate.ExecuteIfBound(ChatMsg);
}

/*void AXBallPlayerControllerBase::Possess(APawn* aPawn)
{
	Super::Possess(aPawn);
	AXBallBase* tmpBall = Cast<AXBallBase>(aPawn);
	if (tmpBall)
	{
		tmpBall->InitPlayer(GetTeam());
	}
}*/

void AXBallPlayerControllerBase::ModifyScore(EScoreType Type)
{
	AXBallPlayerState* XBallPlayerState = GetXBallPlayerState();
	if (!XBallPlayerState)
		return;
	switch (Type)
	{
		case EScoreType::ST_Kill:
			XBallPlayerState->KillScore++;
			return;
		case EScoreType::ST_Dead:
			XBallPlayerState->DeadCount++;
			return;
		case EScoreType::ST_ALL:
			XBallPlayerState->KillScore++;
			XBallPlayerState->DeadCount++;
			return;
	}
}

void AXBallPlayerControllerBase::ClearScore(EScoreType Type)
{
	AXBallPlayerState* XBallPlayerState = GetXBallPlayerState();
	if (!XBallPlayerState)
		return;
	switch (Type)
	{
		case EScoreType::ST_Kill:
			XBallPlayerState->KillScore = 0;
			return;
		case EScoreType::ST_Dead:
			XBallPlayerState->DeadCount = 0;
			return;
		case EScoreType::ST_ALL:
			XBallPlayerState->KillScore = 0;
			XBallPlayerState->DeadCount = 0;
			return;
	}
}

int AXBallPlayerControllerBase::GetScore(EScoreType Type)
{
	AXBallPlayerState* XBallPlayerState = GetXBallPlayerState();
	if (!XBallPlayerState)
		return -1;
	switch (Type)
	{
		case EScoreType::ST_Kill:
			return XBallPlayerState->KillScore;
		case EScoreType::ST_Dead:
			return XBallPlayerState->DeadCount;
		default:
			return -1;
	}
}

bool AXBallPlayerControllerBase::Buy_Internal(TSubclassOf<AActionBase> ActionClass, FString& RetMessage)
{
	if (Coins >= ActionClass->GetDefaultObject<AActionBase>()->GetPrice())
	{
		Coins -= ActionClass->GetDefaultObject<AActionBase>()->GetPrice();
		if (ActionInventory.FindLastByPredicate([=](AActionBase* const Item){return Item->GetClass() == ActionClass;})!=INDEX_NONE)
		{
			RetMessage = TEXT("Already Bought it");
			return false;
		}
		RetMessage = TEXT("Success");
		FActorSpawnParameters tmpSpawnParamters;
		tmpSpawnParamters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AActionBase* ActionInstance = GetWorld()->SpawnActor<AActionBase>(ActionClass, tmpSpawnParamters);
		//ActionInstance->GetRootComponent()->SetVisibility(false, true);
		ActionInventory.Add(ActionInstance);
		return true;
	}
	else
	{
		RetMessage = TEXT("You don't have enought money");
	}
	return false;
}

void AXBallPlayerControllerBase::SwapActionBarItem_Implementation(int IndexA, int IndexB)
{
	if (IndexA==IndexB)
	{
		return;
	}
	AXBallBase* XBallCharacter = Cast<AXBallBase>(GetCharacter());
	if (XBallCharacter)
	{
		AActionBase* ActionA = XBallCharacter->RemoveActionFromBar(IndexA);
		AActionBase* ActionB = XBallCharacter->RemoveActionFromBar(IndexB);
		XBallCharacter->AddActionToBar(IndexB, ActionA);
		XBallCharacter->AddActionToBar(IndexA, ActionB);
	}
	else
	{
		auto* TempActionBarItem = TempActionBar[IndexA];
		TempActionBar[IndexA] = TempActionBar[IndexB];
		TempActionBar[IndexB] = TempActionBarItem;
	}
}

bool AXBallPlayerControllerBase::SwapActionBarItem_Validate(int IndexA, int IndexB)
{
	if (IndexA > 7 || IndexA < 0)
		return false;
	if (IndexB > 7 || IndexB < 0)
		return false;
	return true;
}

void AXBallPlayerControllerBase::ToggleActionInventory()
{
	//Lobby should not open Inventory
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, "Toggle Action Inventory");
	if(!IsInLobby())
	{
		if (ActionInventoryWidget)
		{
			ActionInventoryWidget->RemoveFromParent();
			ActionInventoryWidget = nullptr;
			bShowMouseCursor = false;
			UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx(this, nullptr, EMouseLockMode::LockOnCapture);
		}
		else
		{
			TSubclassOf<UUserWidget> InventoryUMGClass = LoadClass<UUserWidget>(nullptr, TEXT("WidgetBlueprint'/Game/XBall/Blueprints/UMG/Inventory.Inventory_C'"));
			if (InventoryUMGClass)
			{
				UUserWidget* InventoryWidget = UUserWidget::CreateWidgetOfClass(InventoryUMGClass, nullptr, nullptr, this);
				InventoryWidget->AddToViewport(1);
				ActionInventoryWidget = InventoryWidget;
				bShowMouseCursor = true;
				UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx(this, ActionInventoryWidget, EMouseLockMode::LockOnCapture, false);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed To Load InventoryUMG Class"));
				GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Cyan, "Failed To Load InventoryUMG Class");
			}
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Cyan, "Is In Lobby");
	}
}

void AXBallPlayerControllerBase::OpenRank()
{
	if (RankWidget||IsInLobby())
		return;
	TSubclassOf<UUserWidget> RankClass = LoadClass<UUserWidget>(nullptr, TEXT("WidgetBlueprint'/Game/XBall/Blueprints/UMG/Rank.Rank_C'"));
	if (RankClass)
	{
		RankWidget = UUserWidget::CreateWidgetOfClass(RankClass, nullptr, nullptr, this);
		RankWidget->AddToViewport(0);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, "Load RankUMG Class Failed");
	}
}

void AXBallPlayerControllerBase::CloseRank()
{
	if (RankWidget)
	{
		RankWidget->RemoveFromParent();
		RankWidget = nullptr;
	}
}

void AXBallPlayerControllerBase::OpenEscMenu()
{
	if (IsInLobby())
		return;
	if (EscMenuWidget)
	{
		EscMenuWidget->RemoveFromParent();
		EscMenuWidget = nullptr;
	}
	else
	{
		TSubclassOf<UUserWidget> EscMenuClass= LoadClass<UUserWidget>(nullptr, TEXT("WidgetBlueprint'/Game/XBall/Blueprints/UMG/ESCMenu.ESCMenu_C'"));
		if (EscMenuClass)
		{
			EscMenuWidget = UUserWidget::CreateWidgetOfClass(EscMenuClass, nullptr, nullptr, this);
			EscMenuWidget->AddToViewport(3);
		}
	}
}

void AXBallPlayerControllerBase::CloseEscMenu()
{
	if (IsInLobby())
		return;
	if (EscMenuWidget)
	{
		EscMenuWidget->RemoveFromParent();
		EscMenuWidget = nullptr;
	}
}

void AXBallPlayerControllerBase::OpenChatUI()
{
	if (!IsInLobby())
	{
		if (IsValid(ChatUIWidget))
		{
			ChatUIWidget->ProcessEvent(ChatUIWidget->FindFunction("ShowInput"), nullptr);
			return;
		}
	}
}

void AXBallPlayerControllerBase::SetCustomTexture(const TArray<uint8>& TextureData)
{
	if (HasAuthority())
	{
		if (GetXBallPlayerState())
		{
			GetXBallPlayerState()->SetCustomTexture("BaseTexture", TextureData);
			GetXBallPlayerState()->SyncTextureDataToClient();
			if (!IsInLobby())
			{
				GetXBallPlayerState()->GenBaseTexture();
			}
		}
	}
	else
	{
		if (GetXBallPlayerState())
		{
			GetXBallPlayerState()->BaseTextureData = TextureData;
			SendCustomTexture(TextureData.Num());
		}
	}
}

void AXBallPlayerControllerBase::ClientWasKicked_Implementation(const FText& KickReason)
{
	TSubclassOf<UUserWidget> ErrorReturnClass = LoadClass<UUserWidget>(nullptr, TEXT("WidgetBlueprint'/Game/XBall/Blueprints/UMG/ErrorReturnToMainMenu.ErrorReturnToMainMenu_C'"));
	if (ErrorReturnClass)
	{
		FText tmpText = KickReason;
		UUserWidget* ErrorReturn = UUserWidget::CreateWidgetOfClass(ErrorReturnClass, nullptr, nullptr, this);
		ErrorReturn->ProcessEvent(ErrorReturn->FindFunction("SetErrorMsg"), &tmpText);
		ErrorReturn->AddToViewport(999);
	}
}

void AXBallPlayerControllerBase::SetPlayerName_Implementation(const FString& NewName)
{
	PlayerState->SetPlayerName(NewName);
}

bool AXBallPlayerControllerBase::SetPlayerName_Validate(const FString& NewName)
{
	if (NewName.Len() > 128)
		return false;
	return true;
}

void AXBallPlayerControllerBase::ShowResultSync_Implementation(int WinTeam)
{
	TSubclassOf<UUserWidget> ResultClass = LoadClass<UUserWidget>(nullptr, TEXT("WidgetBlueprint'/Game/XBall/Blueprints/UMG/GameResult.GameResult_C'"));
	if (ResultClass)
	{
		UUserWidget* ResultInstance=UUserWidget::CreateWidgetOfClass(ResultClass, nullptr, nullptr, this);
		if (!ResultInstance)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, "Error,Create ResultWidget Failed.");
			return;
		}
		FString WinMsg;
		switch (WinTeam)
		{
			case 1:
				WinMsg = "Red Team Win";
				break;
			case 2:
				WinMsg = "Green Team Win";
				break;
			case 3:
				WinMsg = "Blue Team Win";
				break;
			case 4:
				WinMsg = "Yellow Team Win";
				break;
			default:
				WinMsg = "Nice Game";
				break;
		}
		ResultInstance->ProcessEvent(ResultInstance->FindFunction("SetWinInfo"), &WinMsg);
		ResultInstance->AddToViewport(2);
	}
}

void AXBallPlayerControllerBase::CloseWaitingRespawn_Implementation()
{
	if (WaitRespawn)
	{
		WaitRespawn->RemoveFromParent();
		WaitRespawn = nullptr;
	}
}

void AXBallPlayerControllerBase::ShowWaitingRespawn_Implementation()
{
	if (WaitRespawn)
		return;
	TSubclassOf<UUserWidget> WaitRespawnClass = LoadClass<UUserWidget>(nullptr, TEXT("WidgetBlueprint'/Game/XBall/Blueprints/UMG/WaitRespawn.WaitRespawn_C'"));
	if (WaitRespawnClass)
	{
		WaitRespawn = UUserWidget::CreateWidgetOfClass(WaitRespawnClass, nullptr, nullptr, this);
		WaitRespawn->AddToViewport(-1);
	}
}

void AXBallPlayerControllerBase::InitGameUI_Implementation()
{
	TSubclassOf<UUserWidget> MainUIClass = LoadClass<UUserWidget>(nullptr, TEXT("WidgetBlueprint'/Game/XBall/Blueprints/UMG/GameUI.GameUI_C'"));
	if (MainUIClass)	
	{
		MainUIWidget = UUserWidget::CreateWidgetOfClass(MainUIClass, nullptr, nullptr, this);
		MainUIWidget->AddToViewport(1);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed To Load ActionBar UMG Class"));
	}
}

void AXBallPlayerControllerBase::ShowOK_Implementation(const FString& Title, const FString& Message)
{
	TSubclassOf<UUserWidget> DialogWidget = LoadClass<UUserWidget>(nullptr, TEXT("WidgetBlueprint'/Game/XBall/Blueprints/UMG/Dialog.Dialog_C'"));
	if (DialogWidget)
	{
		UUserWidget* DialogInstance = UUserWidget::CreateWidgetOfClass(DialogWidget, nullptr, nullptr, this);
		struct DialogInfoParamter
		{
			FString Title;
			FString Message;
		} tmpPatamter;
		tmpPatamter.Title = Title;
		tmpPatamter.Message = Message;
		DialogInstance->ProcessEvent(DialogInstance->FindFunction("SetDialogInfoAsOK"),&tmpPatamter);
		DialogInstance->AddToViewport(100);
	}
	else
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, "Error:Failed To Load DialogUMG Class");
}

void AXBallPlayerControllerBase::ShowWarning_Implementation(const FString& Title, const FString& Message)
{
	TSubclassOf<UUserWidget> DialogWidget = LoadClass<UUserWidget>(nullptr, TEXT("WidgetBlueprint'/Game/XBall/Blueprints/UMG/Dialog.Dialog_C'"));
	if (DialogWidget)
	{
		UUserWidget* DialogInstance = UUserWidget::CreateWidgetOfClass(DialogWidget, nullptr, nullptr, this);
		struct DialogInfoParamter
		{
			FString Title;
			FString Message;
		} tmpPatamter;
		tmpPatamter.Title = Title;
		tmpPatamter.Message = Message;
		DialogInstance->ProcessEvent(DialogInstance->FindFunction("SetDialogInfoAsWarning"), &tmpPatamter);
		DialogInstance->AddToViewport(100);
	}
	else
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, "Error:Failed To Load DialogUMG Class");
}

void AXBallPlayerControllerBase::ShowMessage_Implementation(const FString& Title, const FString& Message)
{
	TSubclassOf<UUserWidget> DialogWidget = LoadClass<UUserWidget>(nullptr, TEXT("WidgetBlueprint'/Game/XBall/Blueprints/UMG/Dialog.Dialog_C'"));
	if (DialogWidget)
	{
		UUserWidget* DialogInstance = UUserWidget::CreateWidgetOfClass(DialogWidget, nullptr, nullptr, this);
		struct DialogInfoParamter
		{
			FString Title;
			FString Message;
		} tmpPatamter;
		tmpPatamter.Title = Title;
		tmpPatamter.Message = Message;
		DialogInstance->ProcessEvent(DialogInstance->FindFunction("SetDialogInfoAsMessage"), &tmpPatamter);
		DialogInstance->AddToViewport(100);
	}
	else
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, "Error:Failed To Load DialogUMG Class");
}

void AXBallPlayerControllerBase::MoveActionToInventory_Implementation(int ActionBarIndex)
{
	AXBallBase* XballCharacter = Cast<AXBallBase>(GetCharacter());
	if (XballCharacter)
	{
		AActionBase* TargetAction = XballCharacter->RemoveActionFromBar(ActionBarIndex);
		if (TargetAction)
		{
			ActionInventory.Add(TargetAction);
		}
	}
	else
	{
		if (TempActionBar[ActionBarIndex])
		{
			ActionInventory.Add(TempActionBar[ActionBarIndex]);
			TempActionBar[ActionBarIndex] = nullptr;
		}
	}
}

bool AXBallPlayerControllerBase::MoveActionToInventory_Validate(int ActionBarIndex)
{
	if (ActionBarIndex > 7 || ActionBarIndex < 0)
		return false;
	return true;
}

void AXBallPlayerControllerBase::MoveActionToActionBar_Implementation(AActionBase* ActionInstance, int AddToIndex)
{
	int InventoryIndex;
	if (ActionInventory.Find(ActionInstance, InventoryIndex))
	{
		AXBallBase* XballCharacter= Cast<AXBallBase>(GetCharacter());
		if (XballCharacter)
		{
			AActionBase* ReplacedAction = XballCharacter->AddActionToBar(AddToIndex, ActionInstance);
			if (ReplacedAction)
			{
				ActionInventory.Add(ReplacedAction);
			}
			ActionInventory.RemoveAt(InventoryIndex);
		}
		else
		{
			TempActionBar[AddToIndex] = ActionInstance;
			ActionInventory.RemoveAt(InventoryIndex);
		}
	}
}

bool AXBallPlayerControllerBase::MoveActionToActionBar_Validate(AActionBase* ActionInstance, int AddToIndex)
{
	if (AddToIndex > 7 || AddToIndex < 0||(!ActionInstance))
		return false;
	return true;
}

void AXBallPlayerControllerBase::Buy_Implementation(TSubclassOf<AActionBase> ActionClass)
{
	FString Msg;
	if (Buy_Internal(ActionClass,Msg))
	{
		ShowOK(FString(TEXT("Success")), Msg);
	}
	else
	{
		ShowWarning(FString("Failed"), Msg);
	}
}

bool AXBallPlayerControllerBase::Buy_Validate(TSubclassOf<AActionBase> ActionClass)
{
	return true;
}

void AXBallPlayerControllerBase::UpdatePicData_Implementation(const FString& TextureParamName, const TArray<uint8>& TextureData)
{
	auto* XBallPlayerState = GetXBallPlayerState();
	if (XBallPlayerState)
	{
		XBallPlayerState->SetCustomTexture(TextureParamName, TextureData);
	}
}

bool AXBallPlayerControllerBase::UpdatePicData_Validate(const FString& TextureParamName,const TArray<uint8>& TextureData)
{
	if (TextureData.Num() > 256 * 1024)
		return false;
	return true;
}

bool AXBallPlayerControllerBase::SetPlayerDefaultCharacter_Validate(TSubclassOf<AXBallBase> DefaultCharacter)
{
	return true;
}


void AXBallPlayerControllerBase::SetPlayerDefaultCharacter_Implementation(TSubclassOf<AXBallBase> DefaultCharacter)
{
	PlayerDefaultCharacter = DefaultCharacter;
}