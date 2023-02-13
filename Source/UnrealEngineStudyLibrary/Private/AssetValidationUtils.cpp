// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetValidationUtils.h"
#include "AssetInfoCollector.h"
#include "AssetValidationBlueprint.h"

#include "UnrealEngineStudyLibrary.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/StaticMeshActor.h"
#include "Logging/LogMacros.h"
#include "StringMatchUtils.h"

#define LOCTEXT_NAMESPACE "AssetValidation"

DEFINE_LOG_CATEGORY_STATIC(LogAssetValidationUtils, Verbose, All);

namespace UnrealStudyGlobalVar
{
	TMap<EAssetType, FString> GAssetTypeToStrMap;
	TMap<FString, EAssetType> GStrToAssetTypeMap;
}

namespace AssetValidationTools
{
	void ParseAssetData(const FAssetData& AssetData, FAssetDataInfo& Info)
	{
		FString AssetRefPath = AssetData.AssetClass.ToString() + "'" + AssetData.ObjectPath.ToString() + "'";
		Info.AssetRefPath = AssetRefPath;
		Info.AssetName = AssetData.AssetName.ToString();
		Info.AssetClass = AssetData.AssetClass.ToString();
	}

	FString ParseAssetNameFromType(EAssetType Type)
	{
		using namespace UnrealStudyGlobalVar;
		if (GAssetTypeToStrMap.Contains(Type))
		{
			return *(GAssetTypeToStrMap.Find(Type));
		}
		return FString("");
	}

	EAssetType ParseAssetTypeFromName(FString TypeName)
	{
		using namespace UnrealStudyGlobalVar;
		if (!TypeName.IsEmpty() && GStrToAssetTypeMap.Contains(TypeName))
		{
			return *GStrToAssetTypeMap.Find(TypeName);
		}
		return EAssetType::NotRecognized;
	}

	bool IsAssetTypeSupported(EAssetType Type)
	{
		switch (Type)
		{
		case EAssetType::StaticMesh:
			return true;
		default:
			return false;
		}
	}

	typedef TFunction<bool(FAssetData&)> FAssetFilter;

	void ImportAssetListFromJson(const FString& JsonFilePath, FAssetFilter Filter, TArray<FAssetData>& OutAssets)
	{
		FString JsonStr;
		bool bReadSuccess = FFileHelper::LoadFileToString(JsonStr, *JsonFilePath);
		if (!bReadSuccess)
		{
			UE_LOG(LogAssetValidationUtils, Error, TEXT("Read Json file %s failed."), *JsonFilePath);
			return;
		}

		TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonStr);
		TSharedPtr<FJsonObject> JsonObject;
		bool bParseSuccess = FJsonSerializer::Deserialize(JsonReader, JsonObject);
		if (!bParseSuccess)
		{
			UE_LOG(LogAssetValidationUtils, Error, TEXT("Parse Json file %s failed."), *JsonFilePath);
			return;
		}

		TArray<TSharedPtr<FJsonValue>> JsonAssetData = JsonObject->GetArrayField("AssetData");
		for (int i = 0; i < JsonAssetData.Num(); i ++)
		{
			FAssetData NewAssetData;
			NewAssetData.ObjectPath = FName(*(JsonAssetData[i]->AsObject()->GetStringField("ObjectPath")));
			NewAssetData.PackageName = FName(*(JsonAssetData[i]->AsObject()->GetStringField("PackageName")));
			NewAssetData.PackagePath = FName(*(JsonAssetData[i]->AsObject()->GetStringField("PackagePath")));
			NewAssetData.AssetName = FName(*(JsonAssetData[i]->AsObject()->GetStringField("AssetName")));
			NewAssetData.AssetClass = FName(*(JsonAssetData[i]->AsObject()->GetStringField("AssetClass")));

			if (Filter(NewAssetData))
			{
				OutAssets.Add(NewAssetData);
			}
		}
	}

	void ExportAssetListToJson(const TArray<FAssetData>& InAssets, const FString& JsonPath)
	{
#if WITH_EDITOR
		FString ResultJsonStr;
		// TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> JsonWriter = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&ResultJson);
		TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<TCHAR>::Create(&ResultJsonStr);
		JsonWriter->WriteObjectStart();

		// Write 'PackagePath' Array
		JsonWriter->WriteValue(TEXT("Time"), FDateTime::Now().ToString(TEXT("%y-%m-%d %H:%M:%S")));
		JsonWriter->WriteArrayStart("AssetData");
		for (const FAssetData& AssetData : InAssets)
		{
			JsonWriter->WriteObjectStart();
			JsonWriter->WriteValue(TEXT("ObjectPath"), AssetData.ObjectPath.ToString());
			JsonWriter->WriteValue(TEXT("PackageName"), AssetData.PackageName.ToString());
			JsonWriter->WriteValue(TEXT("PackagePath"), AssetData.PackagePath.ToString());
			JsonWriter->WriteValue(TEXT("AssetName"), AssetData.AssetName.ToString());
			JsonWriter->WriteValue(TEXT("AssetClass"), AssetData.AssetClass.ToString());
			JsonWriter->WriteObjectEnd();
		}
		JsonWriter->WriteArrayEnd();
		JsonWriter->WriteObjectEnd();
		JsonWriter->Close();

		bool bWriteSuccess = FFileHelper::SaveStringToFile(ResultJsonStr, *JsonPath);
		if (bWriteSuccess)
		{
			UE_LOG(LogAssetValidationUtils, Display, TEXT("Write Json file %s success."), *JsonPath);
		}
		else
		{
			UE_LOG(LogAssetValidationUtils, Error, TEXT("Write Json file %s failed."), *JsonPath);
		}
#endif
	}

	void SearchAllAssetList_EditorMode(const FName& PackagePath, EAssetType AssetType, TArray<FAssetData>& OutAssetData)
	{
#if WITH_EDITOR
		FARFilter Filter;
		Filter.PackagePaths.Add(PackagePath);

		if (AssetType != EAssetType::All)
		{
			FString AssetClassName = AssetValidationTools::ParseAssetNameFromType(EAssetType::StaticMesh);
			if (!AssetClassName.IsEmpty())
			{
				Filter.ClassNames.Add(FName(*AssetClassName));
			}
		}

		Filter.bRecursivePaths = true;
		Filter.bRecursiveClasses = true;

		FAssetRegistryModule& AssetRegistryModule =
			FModuleManager::GetModuleChecked<FAssetRegistryModule>("AssetRegistry");
		AssetRegistryModule.Get().SearchAllAssets(true);
		AssetRegistryModule.Get().GetAssets(Filter, OutAssetData);
#endif
		UE_LOG(LogAssetValidationUtils, Verbose, TEXT("SearchAllAssetList_RuntimeMode, %d Asset Found. "),
		       OutAssetData.Num());
	}

	void SearchAllAssetList_RuntimeMode(
		EAssetType AssetType, TArray<FAssetData>& OutAssetData)
	{
		using namespace UnrealStudyGlobalVar;
		ImportAssetListFromJson(SavedAssetJsonFile, [=](FAssetData& AssetData)
		{
			using namespace UnrealStudyGlobalVar;
			if (GAssetTypeToStrMap.Contains(AssetType))
			{
				FString AssetTypeVal = *GAssetTypeToStrMap.Find(AssetType);
				if (AssetTypeVal.Equals(AssetData.AssetClass.ToString()))
				{
					return true;
				}
			}
			return false;
		}, OutAssetData);
		UE_LOG(LogAssetValidationUtils, Verbose, TEXT("SearchAllAssetList_RuntimeMode, %d Asset Found. "),
		       OutAssetData.Num());
	}

	static TWeakObjectPtr<AStaticMeshActor> SpawnedMeshActor = nullptr;

	void SpawnStaticMesh(const UObject* WorldContext, UStaticMesh* StaticMesh, const FTransform& InTransform)
	{
		if (StaticMesh == nullptr)
		{
			return;
		}

		AStaticMeshActor* MyNewActor =
			WorldContext->GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass());
		MyNewActor->SetActorTransform(InTransform);
		MyNewActor->SetMobility(EComponentMobility::Movable);
		UStaticMeshComponent* MeshComponent = MyNewActor->GetStaticMeshComponent();
		if (MeshComponent)
		{
			MeshComponent->SetStaticMesh(StaticMesh);
		}

		if (SpawnedMeshActor.Get())
		{
			// @todo Actor 从场景中删除的最合适写法是啥
			SpawnedMeshActor->Destroy();
		}
		SpawnedMeshActor = MyNewActor;
	}

	void CreateStaticMeshActor(const UObject* WorldContext, const FString& AssetRefPath,
	                           const FTransform& InTransform, FAssetDisplayInfo& OutInfo)
	{
		if (AssetRefPath.IsEmpty())
		{
			UE_LOG(LogAssetValidationUtils, Warning, TEXT("Load Asset failed, asset path empty. "));
			return;
		}

		UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, *AssetRefPath);
		if (Mesh == nullptr)
		{
			UE_LOG(LogAssetValidationUtils, Warning, TEXT("Load Asset failed, asset path %s."), *AssetRefPath);
			return;
		}

		{
			auto AssetPackage = Mesh->GetPackage();
			UE_LOG(LogAssetValidationUtils, Display, TEXT("Load Asset: %s"), *(AssetPackage->FileName.ToString()));
		}

		AssetInfoCollector::CollectAssetInfo<UStaticMesh>(Mesh, OutInfo);
		SpawnStaticMesh(WorldContext, Mesh, InTransform);
	}
}

namespace Test_AssetValidationTools
{
	static TWeakObjectPtr<AStaticMeshActor> CurrentActor = nullptr;

	// @todo GAssetRefsInJsonFile 应当作为 Cache 使用
	static TArray<FString> GAssetRefsInJsonFile;

	static uint8 GAssetRefIndex = 0;

	void Test_SpawnStaticMeshFromJsonList(const UObject* WorldContextObject, const FTransform& Transform)
	{
		// 初始化资产列表
		if (GAssetRefsInJsonFile.Num() == 0)
		{
			// @todo JSON 文件路径，需要修改到插件的 Content 目录，并且可打包
			TArray<FAssetData> AssetData;
			AssetValidationTools::ImportAssetListFromJson(
				UnrealStudyGlobalVar::SavedAssetJsonFile, [](FAssetData& AssetData)
				{
					return true;
				}, AssetData);

			for (const FAssetData& Asset : AssetData)
			{
				FString AssetRefStr = Asset.AssetClass.ToString() + "'" + Asset.ObjectPath.ToString() + "'";
				GAssetRefsInJsonFile.Add(AssetRefStr);
			}
		}

		if (GAssetRefsInJsonFile.Num() <= 0)
		{
			return;
		}

		AStaticMeshActor* MyNewActor = WorldContextObject->GetWorld()->SpawnActor<AStaticMeshActor>(
			AStaticMeshActor::StaticClass());
		MyNewActor->SetActorTransform(Transform);
		MyNewActor->SetMobility(EComponentMobility::Stationary);
		UStaticMeshComponent* MeshComponent = MyNewActor->GetStaticMeshComponent();
		if (MeshComponent)
		{
			UStaticMesh* Mesh = nullptr;
			{
				FString AssetRefPath = GAssetRefsInJsonFile[GAssetRefIndex];
				Mesh = LoadObject<UStaticMesh>(nullptr, *AssetRefPath);
				check(Mesh != nullptr);
				GAssetRefIndex = (GAssetRefIndex + 1) % GAssetRefsInJsonFile.Num();
			}

			if (Mesh != nullptr)
			{
				MeshComponent->SetStaticMesh(Mesh);
			}

			auto TestPkg = Mesh->GetPackage();
			UE_LOG(LogAssetValidationUtils, Display, TEXT("TestAsset::PackageName: %s"),
			       *(TestPkg->FileName.ToString()));
		}

		if (CurrentActor.Get())
		{
			// @todo Actor 从场景中删除的最合适写法是啥
			CurrentActor->Destroy();
		}

		CurrentActor = MyNewActor;
	}
}

UAssetValidationBPLibrary::UAssetValidationBPLibrary(const FObjectInitializer& ObjInitializer): Super(ObjInitializer)
{
}

void UAssetValidationBPLibrary::SearchAssetList(TArray<FAssetDataInfo>& OutAssetInfo, const FString SearchKey,
                                                EAssetType Type)
{
	OutAssetInfo.Empty();
	TArray<FAssetData> FoundAssets;
	// @todo 每次都需要重新搜索，最好做一个缓存
#if WITH_EDITOR
	AssetValidationTools::SearchAllAssetList_EditorMode(FName(TEXT("/Game")), Type, FoundAssets);
#else
	AssetValidationTools::SearchAllAssetList_RuntimeMode(Type, FoundAssets);
#endif

	if (!SearchKey.IsEmpty())
	{
		TArray<FString> AssetSearchSource;
		for (const FAssetData& Asset : FoundAssets)
		{
			FString AssetName = Asset.AssetName.ToString();
			AssetSearchSource.Add(AssetName);
		}
		TArray<int> FilterIndices;
		StringMatchUtils::MatchStrings(AssetSearchSource, SearchKey, FilterIndices);
		for (const int Index : FilterIndices)
		{
			OutAssetInfo.Add(FAssetDataInfo());
			AssetValidationTools::ParseAssetData(FoundAssets[Index], OutAssetInfo.Last());
		}
	}
	else
	{
		for (const FAssetData& AssetData : FoundAssets)
		{
			OutAssetInfo.Add(FAssetDataInfo());
			AssetValidationTools::ParseAssetData(AssetData, OutAssetInfo.Last());
		}
	}
}

void UAssetValidationBPLibrary::SearchAllAssetList(TArray<FAssetDataInfo>& OutAssetInfo)
{
	SearchAssetList(OutAssetInfo, "", EAssetType::All);
}

void UAssetValidationBPLibrary::PackageAssetDataToJson()
{
	TArray<FAssetData> FoundAssets;
#if WITH_EDITOR
	// @todo 不限定资产类型以及搜索路径
	AssetValidationTools::SearchAllAssetList_EditorMode(FName(TEXT("/Game")), EAssetType::All, FoundAssets);
	AssetValidationTools::ExportAssetListToJson(FoundAssets, UnrealStudyGlobalVar::SavedAssetJsonFile);
#endif
}

AStaticMeshActor* UAssetValidationBPLibrary::SpawnActorFromAsset(
	const UObject* WorldContext, const FAssetDataInfo& AssetDataInfo,
	const FTransform& NewTransform, FAssetDisplayInfo& OutDisplayInfo)
{
	OutDisplayInfo.DisplayKeys.Empty();
	OutDisplayInfo.DisplayValues.Empty();

	EAssetType AssetType = AssetValidationTools::ParseAssetTypeFromName(AssetDataInfo.AssetClass);
	switch (AssetType)
	{
	case EAssetType::StaticMesh:
		AssetValidationTools::CreateStaticMeshActor(
			WorldContext, AssetDataInfo.AssetRefPath, NewTransform, OutDisplayInfo);
		return AssetValidationTools::SpawnedMeshActor.Get();
	default:
		UE_LOG(LogAssetValidationUtils, Warning, TEXT("Asset type %s not supported."), *AssetDataInfo.AssetClass);
		break;
	}
	return nullptr;
}

void AssetValidationTools::Initialize()
{
	UnrealStudyGlobalVar::GAssetTypeToStrMap.Add(EAssetType::StaticMesh, FString("StaticMesh"));
	UnrealStudyGlobalVar::GAssetTypeToStrMap.Add(EAssetType::All, FString(""));

	UnrealStudyGlobalVar::GStrToAssetTypeMap.Add(FString("StaticMesh"), EAssetType::StaticMesh);
	UnrealStudyGlobalVar::GStrToAssetTypeMap.Add(FString("All"), EAssetType::All);
}


#undef LOCTEXT_NAMESPACE
