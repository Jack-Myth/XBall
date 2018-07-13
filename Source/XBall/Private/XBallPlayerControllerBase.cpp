// Fill out your copyright notice in the Description page of Project Settings.

#include "XBallPlayerControllerBase.h"
#include "MyBPFuncLib.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "UnrealNetwork.h"
#include "XBallPlayerState.h"

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

AXBallPlayerControllerBase::AXBallPlayerControllerBase()
{
	TempActionBar.SetNum(8);
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

void AXBallPlayerControllerBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
	DOREPLIFETIME(AXBallPlayerControllerBase, ActionInventory)
	DOREPLIFETIME(AXBallPlayerControllerBase, TempActionBar);
}

void AXBallPlayerControllerBase::Possess(APawn* aPawn)
{
	Super::Possess(aPawn);
	/*AXBallBase* tmpBall = Cast<AXBallBase>(aPawn);
	if (tmpBall)
	{
		tmpBall->InitPlayer(GetTeam());
	}*/
}

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
			RetMessage = "已拥有此物品";
		}
		RetMessage = TEXT("购买成功");
		FActorSpawnParameters tmpSpawnParamters;
		tmpSpawnParamters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ActionInventory.Add(GetWorld()->SpawnActor<AActionBase>(ActionClass,tmpSpawnParamters));
		return true;
	}
	else
	{
		RetMessage = TEXT("金钱不足");
	}
	return false;
}

void AXBallPlayerControllerBase::ShowOK_Implementation(const FString& Title, const FString& Message)
{

}

void AXBallPlayerControllerBase::ShowWarning_Implementation(const FString& Title, const FString& Message)
{

}

void AXBallPlayerControllerBase::ShowMessage_Implementation(const FString& Title, const FString& Message)
{

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
		ShowOK(FString("购买成功"), Msg);
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

bool AXBallPlayerControllerBase::SetActionClass_Validate(int Index, TSubclassOf<AActionBase> NewActionClass)
{
	return true;
}

void AXBallPlayerControllerBase::SetPlayerDefaultCharacter_Implementation(TSubclassOf<AXBallBase> DefaultCharacter)
{
	PlayerDefaultCharacter = DefaultCharacter;
}

void AXBallPlayerControllerBase::SetActionClass_Implementation(int Index, TSubclassOf<AActionBase> NewActionClass)
{
	if (Index < 0 || Index>7)
		return;
	ActionClasses[Index] = NewActionClass;
}
