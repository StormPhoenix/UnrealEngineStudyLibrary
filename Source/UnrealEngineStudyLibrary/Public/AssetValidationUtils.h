// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Engine/Classes/Kismet/BlueprintFunctionLibrary.h"
#include "AssetValidationUtils.generated.h"

/**
 * Asset Validation Tools
 */
UCLASS(MinimalAPI, meta=(ScriptName="AssetValidationUtils"))
class UAssetValidationUtils : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()
	
	UFUNCTION(BlueprintCallable, Category = "AssetValidationUtils", meta = (WorldContext = "WorldContextObject"))
	static void SearchAssetList(const UObject* WorldContextObject);
};
