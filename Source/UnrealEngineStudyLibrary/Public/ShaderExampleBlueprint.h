﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Engine/Classes/Kismet/BlueprintFunctionLibrary.h"
#include "ShaderExample.h"
#include "ShaderExampleBlueprint.generated.h"

UCLASS(MinimalAPI, meta=(ScriptName="ShaderTestLibrary"))
class UShaderExampleBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()
	UFUNCTION(BlueprintCallable, Category = "SimpleColorTest", meta = (WorldContext = "WorldContextObject"))
	static void DrawSimpleColorRenderTarget(
		const UObject* WorldContextObject, class UTextureRenderTarget2D* OutputRenderTarget,
		FLinearColor Color, UTexture2D* InTexture, FSimpleShaderParameter ShaderStructData);
};
