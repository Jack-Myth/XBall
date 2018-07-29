// Fill out your copyright notice in the Description page of Project Settings.

#include "MyBPFuncLib.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Object.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialParameterCollection.h"
#include "FileHelper.h"
#include "Engine/Texture2D.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "ActionBase.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "AssetRegistryHelpers.h"
#include "IAssetRegistry.h"
#include "Engine/AssetManager.h"
#ifdef _WIN64
#include "AllowWindowsPlatformTypes.h"
#include <Windows.h>
#include <commdlg.h>
#endif
#include "LobbyGameModeBase.h"

FTimerHandle UMyBPFuncLib::InitAnimationTimeHandle;
TArray<FDelegateHandle> UMyBPFuncLib::OnlineSessionDelegateHandle;

/*void UMyBPFuncLib::SetMapGenerator(UMapGenerator* newMapGenerator)
{
	CoreMapGenerator = newMapGenerator;
}*/

void UMyBPFuncLib::GenWorld(UObject* WorldContextObj, int Ength, int Width, int Height, int32 Seed)
{
	TArray<AActor*> CoreMapGenerators;
	UGameplayStatics::GetAllActorsWithInterface(WorldContextObj, UMapGenerator::StaticClass(), CoreMapGenerators);
	if (!CoreMapGenerators.Num())
		return;
	IMapGenerator::Execute_GenMapBlockInstance(CoreMapGenerators[0], WorldContextObj, Ength, Width, Height,Seed);
	TArray<UMaterialInstanceDynamic*> BlockDMIs = IMapGenerator::Execute_GetDMIs(CoreMapGenerators[0]);
	UMaterialParameterCollection* MPC = LoadObject<UMaterialParameterCollection>(nullptr, TEXT("MaterialParameterCollection'/Game/XBall/Materials/MatData.MatData'"));
	UKismetMaterialLibrary::SetScalarParameterValue(WorldContextObj, MPC, "CubeSize", 50);
	UKismetMaterialLibrary::SetVectorParameterValue(WorldContextObj, MPC, "ScaleOrigin", FLinearColor(FVector(0,0,0)));
	for (int i = 0; i < BlockDMIs.Num(); i++)
	{
		BlockDMIs[i]->SetScalarParameterValue("ScaleRange", 65536);
	}
	//The animation may cause some unpredictable crash. so disable it.
	/*
	WorldContextObj->GetWorld()->GetTimerManager().SetTimer(InitAnimationTimeHandle, [=]()
		{
			int x = BlockDMIs[0]->K2_GetScalarParameterValue("ScaleRange") + 30;
			for (int i = 0; i < BlockDMIs.Num(); i++)
			{
				BlockDMIs[i]->SetScalarParameterValue("ScaleRange", x);
			}
			if (x > 655360)
			{
				WorldContextObj->GetWorld()->GetTimerManager().ClearTimer(InitAnimationTimeHandle);
			}
		}, 0.02f, true, -1);
		*/
}

void UMyBPFuncLib::HitBlockAtLocation(UObject* WorldContextObj, FVector HitWorldLocation, float Damage)
{
	TArray<AActor*> CoreMapGenerators;
	UGameplayStatics::GetAllActorsWithInterface(WorldContextObj, UMapGenerator::StaticClass(), CoreMapGenerators);
	if (!CoreMapGenerators.Num())
		return;
	IMapGenerator::Execute_HitBlock(CoreMapGenerators[0], HitWorldLocation, Damage);
}

void UMyBPFuncLib::HitBlockAtLocationWithNormal(UObject* WorldContextObj, FVector HitWorldLocation, FVector SurfaceNormal, float Damage)
{
	TArray<AActor*> CoreMapGenerators;
	UGameplayStatics::GetAllActorsWithInterface(WorldContextObj, UMapGenerator::StaticClass(), CoreMapGenerators);
	if (!CoreMapGenerators.Num())
		return;
	IMapGenerator::Execute_HitBlockWithNormal(CoreMapGenerators[0], HitWorldLocation,SurfaceNormal, Damage);
}

void UMyBPFuncLib::BreakBlockAtLocation(UObject* WorldContextObj, FVector WorldLocation)
{
	TArray<AActor*> CoreMapGenerators;
	UGameplayStatics::GetAllActorsWithInterface(WorldContextObj, UMapGenerator::StaticClass(), CoreMapGenerators);
	if (!CoreMapGenerators.Num())
		return;
	IMapGenerator::Execute_BreakBlock(CoreMapGenerators[0], WorldLocation);
}

void UMyBPFuncLib::BreakBlockAtRange(UObject* WorldContextObj, FVector WorldLocation, FVector RangeXYZ, bool ShouldIncludeEdge)
{
	TArray<AActor*> CoreMapGenerators;
	UGameplayStatics::GetAllActorsWithInterface(WorldContextObj, UMapGenerator::StaticClass(), CoreMapGenerators);
	if (!CoreMapGenerators.Num())
		return;
	IMapGenerator::Execute_BreakBlockRange(CoreMapGenerators[0], WorldLocation, RangeXYZ, ShouldIncludeEdge);
}

void UMyBPFuncLib::HitBlockAtRange(UObject* WorldContextObj, FVector WorldLocation, FVector RangeXYZ, float MaxDamage)
{
	TArray<AActor*> CoreMapGenerators;
	UGameplayStatics::GetAllActorsWithInterface(WorldContextObj, UMapGenerator::StaticClass(), CoreMapGenerators);
	if (!CoreMapGenerators.Num())
		return;
	IMapGenerator::Execute_HitBlockRange(CoreMapGenerators[0], WorldLocation, RangeXYZ, MaxDamage);
}

void UMyBPFuncLib::SyncMap(UObject* WorldContextObj, const TArray<FBlockInfo>& BlockModifiedInfo)
{
	TArray<AActor*> CoreMapGenerators;
	UGameplayStatics::GetAllActorsWithInterface(WorldContextObj, UMapGenerator::StaticClass(), CoreMapGenerators);
	if (!CoreMapGenerators.Num())
		return;
	IMapGenerator::Execute_SyncMap(CoreMapGenerators[0],BlockModifiedInfo);
}

TArray<FBlockInfo> UMyBPFuncLib::CollectMapModified(UObject* WorldContextObj)
{
	TArray<AActor*> CoreMapGenerators;
	UGameplayStatics::GetAllActorsWithInterface(WorldContextObj, UMapGenerator::StaticClass(), CoreMapGenerators);
	if (!CoreMapGenerators.Num())
		return TArray<FBlockInfo>();
	return IMapGenerator::Execute_CollectBlockModifiedInfo(CoreMapGenerators[0]);
}

void UMyBPFuncLib::ClearMap(UObject* WorldContextObj)
{
	TArray<AActor*> CoreMapGenerators;
	UGameplayStatics::GetAllActorsWithInterface(WorldContextObj, UMapGenerator::StaticClass(), CoreMapGenerators);
	if (CoreMapGenerators.Num())
	{
		IMapGenerator::Execute_ClearMap(CoreMapGenerators[0]);
	}
}

UTexture2D* UMyBPFuncLib::GetTextureFromData(const TArray<uint8>& TextureData)
{
	UTexture2D* OutTex = NULL;
	IImageWrapperModule& imageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> imageWrapper = imageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);

	if (TextureData.Num())
	{
		if (imageWrapper.IsValid() &&
			imageWrapper->SetCompressed(TextureData.GetData(), TextureData.Num()))
		{
			const TArray<uint8>* uncompressedRGBA = NULL;
			if (imageWrapper->GetRaw(ERGBFormat::RGBA, 8, uncompressedRGBA))
			{
				const TArray<FColor> uncompressedFColor = uint8ToFColor(*uncompressedRGBA);
				OutTex = TextureFromImage(
					imageWrapper->GetWidth(),
					imageWrapper->GetHeight(),
					uncompressedFColor,
					true);
			}
		}
	}
	return OutTex;
}

UTexture2D* UMyBPFuncLib::GetLocalTexture(const FString &_TexPath)
{
	UTexture2D* OutTex = NULL;
	TArray<uint8> OutArray;
	if (FFileHelper::LoadFileToArray(OutArray, *_TexPath))
		OutTex = GetTextureFromData(OutArray);
	return OutTex;
}

TArray<FColor> UMyBPFuncLib::uint8ToFColor(const TArray<uint8> origin)
{
	TArray<FColor> uncompressedFColor;
	uint8 auxOrigin;
	FColor auxDst;

	for (int i = 0; i < origin.Num(); i++)
	{
		auxOrigin = origin[i];
		auxDst.R = auxOrigin;
		i++;
		auxOrigin = origin[i];
		auxDst.G = auxOrigin;
		i++;
		auxOrigin = origin[i];
		auxDst.B = auxOrigin;
		i++;
		auxOrigin = origin[i];
		auxDst.A = auxOrigin;
		uncompressedFColor.Add(auxDst);
	}

	return uncompressedFColor;
}

UTexture2D* UMyBPFuncLib::TextureFromImage(const int32 SrcWidth, const int32 SrcHeight, const TArray<FColor>&SrcData, const bool UseAlpha)
{
	UTexture2D* MyScreenshot = UTexture2D::CreateTransient(SrcWidth, SrcHeight, PF_B8G8R8A8);
	uint8* MipData = static_cast<uint8*>(MyScreenshot->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE));
	uint8* DestPtr = NULL;
	const FColor* SrcPtr = NULL;
	for (int32 y = 0; y < SrcHeight; y++)
	{
		DestPtr = &MipData[(SrcHeight - 1 - y)*SrcWidth * sizeof(FColor)];
		SrcPtr = const_cast<FColor*>(&SrcData[(SrcHeight - 1 - y)*SrcWidth]);
		for (int32 x = 0; x < SrcWidth; x++)
		{
			*DestPtr++ = SrcPtr->R;
			*DestPtr++ = SrcPtr->G;
			*DestPtr++ = SrcPtr->B;
			if (UseAlpha)
			{
				*DestPtr++ = SrcPtr->A;
			}
			else
			{
				*DestPtr++ = 0xFF;
			}
			SrcPtr++;
		}
	}

	MyScreenshot->PlatformData->Mips[0].BulkData.Unlock();
	MyScreenshot->UpdateResource();

	return MyScreenshot;
}

UObject* UMyBPFuncLib::LoadObjectNative(UObject* Outer, FString ObjectReference)
{
	return LoadObject<UObject>(Outer, *ObjectReference);
}

UClass* UMyBPFuncLib::LoadClassNative(UObject* Outer, FString ClassPath)
{
	return LoadClass<UObject>(Outer, *ClassPath);
}

UTexture2D* UMyBPFuncLib::GetActionPreview(TSubclassOf<AActionBase> ActionBaseClass)
{
	return ActionBaseClass->GetDefaultObject<AActionBase>()->GetActionIcon();
}

void UMyBPFuncLib::GetActionInfo(TSubclassOf<AActionBase> ActionClass, FString& Title, FString& Intro, int& Price)
{
	AActionBase* AcitonClassDefaultObj = ActionClass->GetDefaultObject<AActionBase>();
	Price = AcitonClassDefaultObj->GetPrice();
	AcitonClassDefaultObj->GetActionInfo(Title, Intro);
}

bool UMyBPFuncLib::CreateOnlineSessionWithName(UObject* WorldContextObj, class APlayerController* HostedController, int PublicConnections, bool UsingLAN, FString ServerName, FOnCreateSessionFinished OnCreateSessionFinished)
{
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		auto Sessions = OnlineSubsystem->GetSessionInterface();
		if (Sessions.IsValid())
		{
			OnlineSessionDelegateHandle.Add(Sessions->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionCompleteDelegate::CreateLambda([OnCreateSessionFinished](FName SessionName, bool IsSucceed)
				{
					OnCreateSessionFinished.ExecuteIfBound(SessionName, IsSucceed);
				})));

			FOnlineSessionSettings Settings;
			Settings.NumPublicConnections = PublicConnections;
			Settings.bShouldAdvertise = true;
			Settings.bAllowJoinInProgress = true;
			Settings.bIsLANMatch = UsingLAN;
			Settings.bUsesPresence = true;
			Settings.bAllowJoinViaPresence = true;
			Settings.Set(TEXT("CustomServerName"), ServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

			Sessions->CreateSession(HostedController->NetPlayerIndex, NAME_GameSession, Settings);

			// OnCreateCompleted will get called, nothing more to do now
			return true;
		}
	}
		return false;
}

void UMyBPFuncLib::ClearOnlineSubsystemNotifyDelegate()
{
	for (int i = 0; i < OnlineSessionDelegateHandle.Num(); i++)
	{
		IOnlineSubsystem::Get()->GetSessionInterface()->ClearOnCreateSessionCompleteDelegate_Handle(OnlineSessionDelegateHandle[i]);
	}
	OnlineSessionDelegateHandle.Empty();
}

TArray<UClass*> UMyBPFuncLib::SearchBPClassByPath(FName AssetsPath, TSubclassOf<UObject> TargetClass)
{
	TArray<FAssetData> Assets;
	TArray<UClass*> TargetClasses;
	UAssetManager::Get().GetAssetRegistry().GetAssetsByPath(AssetsPath, Assets, true);
	for (auto it= Assets.CreateIterator();it;++it)
	{
		const FString* GeneratedClassMetaData = it->TagsAndValues.Find("GeneratedClass");
		FString ClassPath= FPackageName::ExportTextPathToObjectPath(*GeneratedClassMetaData);
		UClass* AssetClass = LoadClass<UObject>(nullptr,*ClassPath);
		if(AssetClass&&AssetClass->IsChildOf(TargetClass))
			TargetClasses.Add(AssetClass);
	}
	return TargetClasses;
}

FString UMyBPFuncLib::GetPlatformOpenFileName(FString Title, FString DefaultPath, FString Filter)
{
#ifdef _WIN64
	OPENFILENAME ofn = {0};
	TCHAR* FilterChars = new TCHAR[Filter.Len() + 2];
	for (int i = 0; i < Filter.Len(); i++)
	{
		if (Filter[i]==TEXT('|'))
		{
			FilterChars[i] = 0;
		}
		else
		{
			FilterChars[i] = Filter[i];
		}
	}
	FilterChars[Filter.Len() + 1] = 0;
	FilterChars[Filter.Len()] = 0;
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.lpstrFilter = FilterChars;
	ofn.lpstrTitle = *Title;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrInitialDir = *DefaultPath;
	TCHAR FilePath[1024] = {NULL};
	ofn.lpstrFile = FilePath;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_EXPLORER;
	if (::GetOpenFileName(&ofn))
	{
		delete FilterChars;
		return FilePath;
	}
	delete FilterChars;
#endif
	return "";
}

TArray<uint8> UMyBPFuncLib::LoadFileAsBytes(FString FilePath)
{
	TArray<uint8> FileData;
	FFileHelper::LoadFileToArray(FileData, *FilePath);
	return FileData;
}

FString UMyBPFuncLib::GetCustomServerName(const FBlueprintSessionResult& SessionResult)
{
	FString Name;
	SessionResult.OnlineResult.Session.SessionSettings.Get<FString>(TEXT("CustomServerName"), Name);
	return Name;
}

void UMyBPFuncLib::InitDedicatedServer(class ALobbyGameModeBase* LobbyGameMode)
{
	int i = 0;
	for (;i<__argc;i++)
	{
		if(FString(__argv[i]).Equals("-InitJson",ESearchCase::IgnoreCase))
			break;
	}
	if (i<__argc-1)
	{
		FString JsonPath = __argv[i + 1];
		FString JsonDataStr;
		if (!FFileHelper::LoadFileToString(JsonDataStr, *JsonPath))
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid Json File:%s"), *JsonPath);
			exit(0);
		}
		TSharedRef<TJsonReader<TCHAR>> ResultJsonReader = TJsonReaderFactory<TCHAR>::Create(JsonDataStr);
		TSharedPtr<FJsonObject> JsonParsed = MakeShareable(new FJsonObject());
		FJsonSerializer::Deserialize(ResultJsonReader, JsonParsed);
		LobbyGameMode->bIsTeamPlay = JsonParsed->GetBoolField("TeamPlay");
		int TempNumber;
		if (JsonParsed->TryGetNumberField("InitCoins", TempNumber))
			LobbyGameMode->InitMoney = TempNumber;
		if (JsonParsed->TryGetNumberField("MaxPlayer", TempNumber))
			LobbyGameMode->MaxPlayer = TempNumber;
		if (JsonParsed->TryGetNumberField("TargetScore", TempNumber))
			LobbyGameMode->TargetScore = TempNumber;
		if (JsonParsed->TryGetNumberField("MaxE", TempNumber))
			LobbyGameMode->MaxE = TempNumber;
		if (JsonParsed->TryGetNumberField("MaxW", TempNumber))
			LobbyGameMode->MaxW = TempNumber;
		if (JsonParsed->TryGetNumberField("MaxH", TempNumber))
			LobbyGameMode->MaxH = TempNumber;
	}
	LobbyGameMode->LoadGameMapSeamless("Minimal_Default");
}
