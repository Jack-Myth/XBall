// Fill out your copyright notice in the Description page of Project Settings.

#include "XBallPlayerControllerBase.h"
#include "MyBPFuncLib.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "UnrealNetwork.h"
#include "XBallPlayerState.h"
#include "UserWidget.h"
#include "WidgetBlueprintLibrary.h"

void AXBallPlayerControllerBase::ReGenOldMap_Implementation(UClass* MapGenerator,int MaxEngth, int MaxWidth, int MaxHeight, int32 Seed,const TArray<FBlockInfo>& BlockInfo)
{
	GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Blue, "Controller Executed RPC");
	if (this->HasAuthority())
	{
		GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, "Warning:Server Is Executeing the ReGenOldMap()!");
	}
	else
		GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Green, "OK:Client Is Executeing the ReGenOldMap().");
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
}

AXBallPlayerControllerBase::AXBallPlayerControllerBase()
{
	TempActionBar.SetNum(8,false);
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
		tmpPlayerState->Team=NewTeam;
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

void AXBallPlayerControllerBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AXBallPlayerControllerBase, ActionInventory)
	DOREPLIFETIME(AXBallPlayerControllerBase, TempActionBar);
	DOREPLIFETIME(AXBallPlayerControllerBase, bIsInLobby);
	DOREPLIFETIME(AXBallPlayerControllerBase, Coins);
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
				InventoryWidget->AddToViewport();
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

}

void AXBallPlayerControllerBase::CloseRank()
{

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