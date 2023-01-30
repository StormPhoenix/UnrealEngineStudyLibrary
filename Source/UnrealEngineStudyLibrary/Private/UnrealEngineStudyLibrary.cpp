// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnrealEngineStudyLibrary.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FUnrealEngineStudyLibraryModule"

void FUnrealEngineStudyLibraryModule::StartupModule()
{
	FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("UnrealEngineStudyLibrary"))->GetBaseDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/UnrealEngineStudyLibrary"), PluginShaderDir);
}

void FUnrealEngineStudyLibraryModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUnrealEngineStudyLibraryModule, UnrealEngineStudyLibrary)