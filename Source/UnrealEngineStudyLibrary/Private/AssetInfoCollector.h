#pragma once

#include "AssetValidationUtils.h"
#include "Engine/StaticMesh.h"

namespace AssetInfoCollector
{
	template <typename AssetEntityType>
	void CollectAssetInfo(const AssetEntityType* InAssetEntity, FAssetDisplayInfo& OutDisplayInfo);

	template <>
	void CollectAssetInfo(const UStaticMesh* InStaticMesh, FAssetDisplayInfo& OutDisplayInfo)
	{
		if (InStaticMesh != nullptr)
		{
			{
				FString Key = "Approx Size";
				FString Value = FText::Format(
					NSLOCTEXT("AssetValidation", "ApproxSize_F", "{0}x{1}x{2}"),
					FText::AsNumber(int32(InStaticMesh->GetBounds().BoxExtent.X * 2.0f)),
					FText::AsNumber(int32(InStaticMesh->GetBounds().BoxExtent.Y * 2.0f)),
					FText::AsNumber(int32(InStaticMesh->GetBounds().BoxExtent.Z * 2.0f))).ToString();
				OutDisplayInfo.AddInfo(Key, Value);
			}
		}
	}
};
