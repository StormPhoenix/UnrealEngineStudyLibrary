// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetValidationUtils.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Logging/LogMacros.h"

#define LOCTEXT_NAMESPACE "AssetValidation"

DEFINE_LOG_CATEGORY_STATIC(LogAssetValidationUtils, Verbose, All);

namespace AssetValidationTools
{
	void SearchAllAssetList()
	{
		// @todo 检查 WITH_EDITOR 宏
		
		// @todo 测试内容，检查资产搜索是否能在 Android 平台生效
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(
			TEXT("AssetRegistry"));
		// const FName PackageName = "/UnrealEngineStudyLibrary/Blueprint/BP_ShaderExample.BP_ShaderExample";
		const FName PackageName = "/UnrealEngineStudyLibrary/Blueprint/BP_ShaderExample";
		TArray<FName> Dependencies;
		AssetRegistryModule.Get().GetDependencies(PackageName, Dependencies);

		UE_LOG(LogAssetValidationUtils, Display, TEXT("TestAsset::Dependencies: %d"), Dependencies.Num());
		for (auto DependsIt = Dependencies.CreateConstIterator(); DependsIt; ++DependsIt)
		{
			FString DependencyName = (*DependsIt).ToString();
			UE_LOG(LogAssetValidationUtils, Display, TEXT("TestAsset::DependencyName: %s"), *DependencyName);
		}
		// @todo end

		UE_LOG(LogAssetValidationUtils, Display, TEXT("AssetValidationTools::SearchAllAssetList()"));
	}
}

UAssetValidationUtils::UAssetValidationUtils(const FObjectInitializer& ObjInitializer): Super(ObjInitializer)
{
}

void UAssetValidationUtils::SearchAssetList(const UObject* WorldContextObject)
{
#if WITH_EDITOR
	AssetValidationTools::SearchAllAssetList();
#elif
	AssetValidationTools::SearchAllAssetList();
#endif
}

#undef LOCTEXT_NAMESPACE
