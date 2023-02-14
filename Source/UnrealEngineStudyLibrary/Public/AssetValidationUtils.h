#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "AssetValidationUtils.generated.h"

// #define GET_FORMAT_PRINT_MESSAGE(InNamespace) \
// 	FText::Format(NSLOCTEXT("AssetValidation", TEXT(InNamespace), "{0}"),);

USTRUCT(BlueprintType)
struct FAssetDataInfo
{
	/**
	 * 资产元数据
	 */
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AssetValidationUtils")
	FString AssetRefPath;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AssetValidationUtils")
	FString AssetName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AssetValidationUtils")
	FString AssetClass;
};

USTRUCT(BlueprintType)
struct FAssetDisplayInfo
{
	/**
	 * 资产可展示信息、必要日志警告等
	 */
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AssetValidationUtils")
	TArray<FString> DisplayKeys;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AssetValidationUtils")
	TArray<FString> DisplayValues;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AssetValidationUtils")
	FVector ApproxSize{0, 0, 0};

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AssetValidationUtils")
	TArray<FString> PrintMessages;
	
	void AddInfo(FString Key, FString Value)
	{
		DisplayKeys.Add(Key);
		DisplayValues.Add(Value);
	}

	void AddPrintMessage(FString Message)
	{
		PrintMessages.Add(Message);
	}

	void Clear()
	{
		DisplayKeys.Empty();
		DisplayValues.Empty();
		PrintMessages.Empty();
	}
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

namespace AssetValidationTools
{
	static void Initialize();
}
