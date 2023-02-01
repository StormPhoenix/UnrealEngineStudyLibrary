// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetValidationUtils.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/StaticMeshActor.h"
#include "Logging/LogMacros.h"

#define LOCTEXT_NAMESPACE "AssetValidation"

DEFINE_LOG_CATEGORY_STATIC(LogAssetValidationUtils, Verbose, All);

namespace Test_AssetValidationTools
{
	// static TArray<FString> GTestAssetPathList();
	static TArray<FString> GTestAssetPathList = {
		TEXT("StaticMesh'/Game/HXAssets/Standardfile/Scene/Prop/Prop_YW_Damen_001_A.Prop_YW_Damen_001_A'"),
		TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"),
		TEXT("StaticMesh'/Engine/BasicShapes/Cylinder.Cylinder'")
	};

	static uint8 GTestAssetIndex = 0;

	static AStaticMeshActor* CurrentActor = nullptr;

	void Test_SpawnStaticMesh(const UObject* WorldContextObject, const FTransform& Transform)
	{
		AStaticMeshActor* MyNewActor = WorldContextObject->GetWorld()->SpawnActor<AStaticMeshActor>(
			AStaticMeshActor::StaticClass());
		MyNewActor->SetActorTransform(Transform);
		MyNewActor->SetMobility(EComponentMobility::Stationary);
		UStaticMeshComponent* MeshComponent = MyNewActor->GetStaticMeshComponent();
		if (MeshComponent)
		{
			UStaticMesh* Mesh = nullptr;
			{
				Mesh = LoadObject<UStaticMesh>(nullptr, *(GTestAssetPathList[GTestAssetIndex]));
				check(Mesh != nullptr);
				GTestAssetIndex = (GTestAssetIndex + 1) % GTestAssetPathList.Num();
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

	void Test_ExportAssetListToJson(const TArray<FAssetData>& InAsset)
	{
#if WITH_EDITOR
#endif
	}
}

namespace AssetValidationTools
{
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
	Test_AssetValidationTools::Test_ExportAssetListToJson(FoundAssets);
#endif
}

#undef LOCTEXT_NAMESPACE
