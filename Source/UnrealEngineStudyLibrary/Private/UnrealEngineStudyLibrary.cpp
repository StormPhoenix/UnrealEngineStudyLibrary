// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnrealEngineStudyLibrary.h"
#include "Interfaces/IPluginManager.h"
#include "AssetValidationUtils.h"

#define LOCTEXT_NAMESPACE "FUnrealEngineStudyLibraryModule"

namespace UnrealStudyGlobalVar
{
	FString PluginDir = "";
	FString PluginContentDir = "";
	FString SavedAssetJsonFile = "";
}

void FUnrealEngineStudyLibraryModule::StartupModule()
{
	FString PluginShaderDir = FPaths::Combine(
		IPluginManager::Get().FindPlugin(TEXT("UnrealEngineStudyLibrary"))->GetBaseDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/UnrealEngineStudyLibrary"), PluginShaderDir);

	// 初始化全局变量
	UnrealStudyGlobalVar::PluginDir =
		FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("UnrealEngineStudyLibrary"));
	UnrealStudyGlobalVar::PluginContentDir = FPaths::Combine(UnrealStudyGlobalVar::PluginDir, TEXT("Content"));
	UnrealStudyGlobalVar::SavedAssetJsonFile = FPaths::Combine(FPaths::ProjectContentDir() + FString("AssetValidation/SavedAssetsList.json"));

	UAssetValidationUtils::Initialize();
}

void FUnrealEngineStudyLibraryModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FUnrealEngineStudyLibraryModule, UnrealEngineStudyLibrary)
