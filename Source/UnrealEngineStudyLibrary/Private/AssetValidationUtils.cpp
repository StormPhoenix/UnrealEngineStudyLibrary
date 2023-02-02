// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetValidationUtils.h"

#include "UnrealEngineStudyLibrary.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/StaticMeshActor.h"
#include "Logging/LogMacros.h"

#define LOCTEXT_NAMESPACE "AssetValidation"

DEFINE_LOG_CATEGORY_STATIC(LogAssetValidationUtils, Verbose, All);

namespace AssetValidationTools
{
	void ImportAssetListFromJson(const FString& JsonFilePath, TArray<FAssetData>& OutAssets)
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
			OutAssets.Add(FAssetData());
			OutAssets.Last().ObjectPath = FName(*(JsonAssetData[i]->AsObject()->GetStringField("ObjectPath")));
			OutAssets.Last().PackageName = FName(*(JsonAssetData[i]->AsObject()->GetStringField("PackageName")));
			OutAssets.Last().PackagePath = FName(*(JsonAssetData[i]->AsObject()->GetStringField("PackagePath")));
			OutAssets.Last().AssetName = FName(*(JsonAssetData[i]->AsObject()->GetStringField("AssetName")));
			OutAssets.Last().AssetClass = FName(*(JsonAssetData[i]->AsObject()->GetStringField("AssetClass")));
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

	void SearchAllAssetList_EditorMode(const FName& PackagePath, const FName& ClassName,
	                                   TArray<FAssetData>& OutAssetData)
	{
#if WITH_EDITOR
		FARFilter Filter;
		Filter.PackagePaths.Add(PackagePath);
		Filter.ClassNames.Add(ClassName);
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

	void SearchAllAssetList_RuntimeMode(const FName& PackagePath, const FName& ClassName,
	                                    TArray<FAssetData>& OutAssetData)
	{
		ImportAssetListFromJson(UnrealStudyGlobalVar::SavedAssetJsonFile, OutAssetData);
		UE_LOG(LogAssetValidationUtils, Verbose, TEXT("SearchAllAssetList_RuntimeMode, %s Asset Found. "),
		       OutAssetData.Num());
	}
}

namespace Test_AssetValidationTools
{
	static TWeakObjectPtr<AStaticMeshActor> CurrentActor = nullptr;

	void Test_SpawnStaticMesh(const UObject* WorldContextObject, const FString& AssetRefPath,
	                          const FTransform& Transform)
	{
		if (AssetRefPath.IsEmpty())
		{
			UE_LOG(LogAssetValidationUtils, Warning, TEXT("Load Asset failed, asset path empty. "));
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
				Mesh = LoadObject<UStaticMesh>(nullptr, *AssetRefPath);
				// @todo 需要判空
				check(Mesh != nullptr);
			}

			if (Mesh != nullptr)
			{
				MeshComponent->SetStaticMesh(Mesh);
			}
			else
			{
				UE_LOG(LogAssetValidationUtils, Warning, TEXT("Load Asset failed, asset path %s."), *AssetRefPath);
				return;
			}

			auto AssetPackage = Mesh->GetPackage();
			UE_LOG(LogAssetValidationUtils, Display, TEXT("Load Asset: %s"), *(AssetPackage->FileName.ToString()));
		}

		if (CurrentActor.Get())
		{
			// @todo Actor 从场景中删除的最合适写法是啥
			CurrentActor->Destroy();
		}

		CurrentActor = MyNewActor;
	}

	static TArray<FString> GAssetRefsInJsonFile;

	static uint8 GAssetRefIndex = 0;

	void Test_SpawnStaticMeshFromJsonList(const UObject* WorldContextObject, const FTransform& Transform)
	{
		// 初始化资产列表
		if (GAssetRefsInJsonFile.Num() == 0)
		{
			// @todo JSON 文件路径，需要修改到插件的 Content 目录，并且可打包
			TArray<FAssetData> AssetData;
			AssetValidationTools::ImportAssetListFromJson(UnrealStudyGlobalVar::SavedAssetJsonFile, AssetData);

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

UAssetValidationUtils::UAssetValidationUtils(const FObjectInitializer& ObjInitializer): Super(ObjInitializer)
{
}

void UAssetValidationUtils::Test_SpawnStaticMesh(const UObject* WorldContextObject, const FString& AssetRefPath,
                                                 const FTransform& NewTransform)
{
	Test_AssetValidationTools::Test_SpawnStaticMesh(WorldContextObject, AssetRefPath, NewTransform);
}

void UAssetValidationUtils::SearchAssetList(TArray<FAssetDataInfo>& OutAssetInfo)
{
	TArray<FAssetData> FoundAssets;
	// @todo 不限定资产类型以及搜索路径
#if WITH_EDITOR
	AssetValidationTools::SearchAllAssetList_EditorMode(FName(TEXT("/Game")), FName(TEXT("StaticMesh")), FoundAssets);
#else
	AssetValidationTools::SearchAllAssetList_RuntimeMode(FName(TEXT("/Game")), FName(TEXT("StaticMesh")), FoundAssets);
#endif

	for (const FAssetData& Asset : FoundAssets)
	{
		OutAssetInfo.Add(FAssetDataInfo());
		FString AssetRefPath = Asset.AssetClass.ToString() + "'" + Asset.ObjectPath.ToString() + "'";
		OutAssetInfo.Last().AssetRefPath = AssetRefPath;
		OutAssetInfo.Last().AssetName = Asset.AssetName.ToString();
		OutAssetInfo.Last().AssetClass = Asset.AssetClass.ToString();
	}
}

void UAssetValidationUtils::PackageAssetDataToJson()
{
	TArray<FAssetData> FoundAssets;
#if WITH_EDITOR
	// @todo 不限定资产类型以及搜索路径
	AssetValidationTools::SearchAllAssetList_EditorMode(FName(TEXT("/Game")), FName(TEXT("StaticMesh")), FoundAssets);
	AssetValidationTools::ExportAssetListToJson(FoundAssets, UnrealStudyGlobalVar::SavedAssetJsonFile);
#endif
}


#undef LOCTEXT_NAMESPACE
