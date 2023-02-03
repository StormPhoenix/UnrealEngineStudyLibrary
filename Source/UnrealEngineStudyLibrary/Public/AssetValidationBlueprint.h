﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Engine/Classes/Kismet/BlueprintFunctionLibrary.h"
#include "AssetValidationBlueprint.generated.h"


USTRUCT(BlueprintType)
struct FAssetDataInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AssetValidationUtils")
	FString AssetRefPath;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AssetValidationUtils")
	FString AssetName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AssetValidationUtils")
	FString AssetClass;
};

UENUM(BlueprintType)
enum class EAssetType : uint8
{
	StaticMesh UMETA(DisplayName="StaticMesh"),
	All UMETA(DisplayName="All"),
	NotRecognized UMETA(DisplayName="NotRecognized"),
};

namespace UnrealStudyGlobalVar
{
	extern TMap<EAssetType, FString> GAssetTypeToStrMap;
	extern TMap<FString, EAssetType> GStrToAssetTypeMap;
}

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

	// @todo 做一个单独的类，拉取出来，不要放在 BP 代码里面
	static void Initialize();
};