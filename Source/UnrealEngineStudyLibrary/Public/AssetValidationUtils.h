#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
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
