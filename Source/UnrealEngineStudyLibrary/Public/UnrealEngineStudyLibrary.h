// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

namespace UnrealStudyGlobalVar
{
	extern FString PluginDir;
	extern FString PluginContentDir;
	extern FString SavedAssetJsonFile;
}

class FUnrealEngineStudyLibraryModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
