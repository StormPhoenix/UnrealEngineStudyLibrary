// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Engine/Classes/Kismet/BlueprintFunctionLibrary.h"
#include "ShaderTestBlueprintLibrary.generated.h"

UCLASS(MinimalAPI, meta=(ScriptName="ShaderTestLibrary"))
class UShaderTestBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()
	
	UFUNCTION(BlueprintCallable, Category = "SimpleColorTest", meta = (WorldContext = "WorldContextObject"))
	static void DrawSimpleColorRenderTarget(
		const UObject* WorldContextObject, class UTextureRenderTarget2D* OutputRenderTarget,
		AActor* Actor, FLinearColor Color);
};
