// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "AssetValidationUtils.h"
#include "Engine/Classes/Kismet/BlueprintFunctionLibrary.h"
#include "AssetValidationBlueprint.generated.h"

/**
 * Asset Validation Tools
 */
UCLASS(MinimalAPI, meta=(ScriptName="AssetValidationUtils"))
class UAssetValidationBPLibrary: public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()
	UFUNCTION(BlueprintCallable, Category = "AssetValidationUtils", meta = (WorldContext = "WorldContextObject"))
	static void SearchAllAssetList(TArray<FAssetDataInfo>& OutAssetInfo);

	UFUNCTION(BlueprintCallable, Category = "AssetValidationUtils", meta = (WorldContext = "WorldContextObject"))
	static void SearchAssetList(TArray<FAssetDataInfo>& OutAssetInfo, const FString SearchKey = "",
	                            EAssetType SearchType = EAssetType::All);

	UFUNCTION(BlueprintCallable, Category = "AssetValidationUtils", meta = (WorldContext = "WorldContextObject"))
	static void PackageAssetDataToJson();

	UFUNCTION(BlueprintCallable, Category = "AssetValidationUtils", meta = (WorldContext = "WorldContextObject"))
	static void SpawnActorFromAsset(const UObject* WorldContextObject, const FAssetDataInfo& AssetDataInfo,
	                                const FTransform& NewTransform);
};
