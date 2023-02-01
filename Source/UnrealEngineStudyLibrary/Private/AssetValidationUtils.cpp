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

	void SearchAllAssetList(const FName& PackagePath, const FName& ClassName, TArray<FAssetData>& OutAssets)
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
		AssetRegistryModule.Get().GetAssets(Filter, OutAssets);

		UE_LOG(LogAssetValidationUtils, Display, TEXT("SearchAllAssetList Method: %d asset found"), OutAssets.Num());
#endif
	}
}

namespace Test_AssetValidationTools
{
	static TArray<FString> GAssetRefsInJsonFile;

	static uint8 GAssetRefIndex = 0;

	static AStaticMeshActor* CurrentActor = nullptr;

	void Test_SpawnStaticMesh(const UObject* WorldContextObject, const FTransform& Transform)
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

		if (CurrentActor != nullptr)
		{
			CurrentActor->Destroy();
		}

		CurrentActor = MyNewActor;
	}
}

UAssetValidationUtils::UAssetValidationUtils(const FObjectInitializer& ObjInitializer): Super(ObjInitializer)
{
}

void UAssetValidationUtils::Test_SpawnStaticMesh(const UObject* WorldContextObject, const FTransform& NewTransform)
{
	Test_AssetValidationTools::Test_SpawnStaticMesh(WorldContextObject, NewTransform);
}

void UAssetValidationUtils::SearchAssetList(const UObject* WorldContextObject)
{
#if WITH_EDITOR
	TArray<FAssetData> FoundAssets;
	AssetValidationTools::SearchAllAssetList(FName(TEXT("/Game")), FName(TEXT("StaticMesh")), FoundAssets);

	// @todo 修改成插件路径，并实现打包
	AssetValidationTools::ExportAssetListToJson(FoundAssets, UnrealStudyGlobalVar::SavedAssetJsonFile);
#endif
}

#undef LOCTEXT_NAMESPACE
