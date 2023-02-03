#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ShaderExample.generated.h"

USTRUCT(BlueprintType, meta = (ScriptName = "SimpleShaderExample"))
struct FSimpleShaderParameter
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, meta = (WorldContext = "WorldContextObject"))
	FLinearColor Color1;
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, meta = (WorldContext = "WorldContextObject"))
	FLinearColor Color2;
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, meta = (WorldContext = "WorldContextObject"))
	FLinearColor Color3;
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, meta = (WorldContext = "WorldContextObject"))
	FLinearColor Color4;
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, meta = (WorldContext = "WorldContextObject"))
	int32 ColorIndex;
};

namespace ShaderExample
{
	BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FSimpleUniformStructParameters,)
		SHADER_PARAMETER(FVector4f, Color1)
		SHADER_PARAMETER(FVector4f, Color2)
		SHADER_PARAMETER(FVector4f, Color3)
		SHADER_PARAMETER(FVector4f, Color4)
		SHADER_PARAMETER(uint32, ColorIndex)
	END_GLOBAL_SHADER_PARAMETER_STRUCT()
};
