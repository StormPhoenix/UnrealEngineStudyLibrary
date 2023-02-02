// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Engine/Classes/Kismet/BlueprintFunctionLibrary.h"
#include "AssetValidationUtils.generated.h"


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

/**
 * Asset Validation Tools
 */
UCLASS(MinimalAPI, meta=(ScriptName="AssetValidationUtils"))
class UAssetValidationUtils : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()
	UFUNCTION(BlueprintCallable, Category = "AssetValidationUtils", meta = (WorldContext = "WorldContextObject"))
	static void SearchAssetList(TArray<FAssetDataInfo>& OutAssetInfo);

	UFUNCTION(BlueprintCallable, Category = "AssetValidationUtils", meta = (WorldContext = "WorldContextObject"))
	static void PackageAssetDataToJson();

	UFUNCTION(BlueprintCallable, Category = "AssetValidationUtils", meta = (WorldContext = "WorldContextObject"))
	static void Test_SpawnStaticMesh(const UObject* WorldContextObject, const FString& AssetRefPath,
	                                 const FTransform& NewTransform);
};
