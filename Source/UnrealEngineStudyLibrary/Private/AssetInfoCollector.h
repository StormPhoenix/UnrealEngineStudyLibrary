#pragma once

#include "AssetValidationUtils.h"
#include "Engine/StaticMesh.h"

namespace AssetInfoCollector
{
	template <typename AssetEntityType>
	void CollectAssetInfo(AssetEntityType* InAssetEntity, FAssetDisplayInfo& OutDisplayInfo);

	template <>
	void CollectAssetInfo(UStaticMesh* StaticMesh, FAssetDisplayInfo& OutDisplayInfo)
	{
		if (StaticMesh != nullptr)
		{
			if (StaticMesh->GetRenderData() != nullptr)
			{
				FString Value;
				const int32 LODNum = StaticMesh->GetRenderData()->LODResources.Num();
				const int32 MatNum = StaticMesh->GetStaticMaterials().Num();
				const FStaticMeshLODResources& LODModel = StaticMesh->GetRenderData()->LODResources[0];
				const int32 NumVerts = LODModel.GetNumVertices();
				const int32 NumTris = LODModel.GetNumTriangles();
				const int32 NumUVChannels = StaticMesh->GetNumUVChannels(0);

				Value = FText::Format(
					NSLOCTEXT("AssetValidation", "Vertices_F", "{0}"),
					FText::AsNumber(NumVerts)).ToString();
				OutDisplayInfo.AddInfo(FString("Vertices"), Value);
				
				Value = FText::Format(
					NSLOCTEXT("AssetValidation", "Triangles_F", "{0}"),
					FText::AsNumber(NumTris)).ToString();
				OutDisplayInfo.AddInfo(FString("Triangels"), Value);

				Value = FText::Format(
					NSLOCTEXT("AssetValidation", "LODs_F", "{0}"),
					FText::AsNumber(LODNum)).ToString();
				OutDisplayInfo.AddInfo(FString("LODs"), Value);
				
				Value = FText::Format(
					NSLOCTEXT("AssetValidation", "Materials_F", "{0}"),
					FText::AsNumber(MatNum)).ToString();
				OutDisplayInfo.AddInfo(FString("Materials"), Value);
				
				Value = FText::Format(
					NSLOCTEXT("AssetValidation", "UVChannels_F", "{0}"),
					FText::AsNumber(NumUVChannels)).ToString();
				OutDisplayInfo.AddInfo(FString("UV Channels"), Value);
			}

			{
				FString Key = "Approx Size";
				FString Value = FText::Format(
					NSLOCTEXT("AssetValidation", "ApproxSize_F", "{0}x{1}x{2}"),
					FText::AsNumber(int32(StaticMesh->GetBounds().BoxExtent.X * 2.0f)),
					FText::AsNumber(int32(StaticMesh->GetBounds().BoxExtent.Y * 2.0f)),
					FText::AsNumber(int32(StaticMesh->GetBounds().BoxExtent.Z * 2.0f))).ToString();
				OutDisplayInfo.AddInfo(Key, Value);
			}
		}
	}
};
