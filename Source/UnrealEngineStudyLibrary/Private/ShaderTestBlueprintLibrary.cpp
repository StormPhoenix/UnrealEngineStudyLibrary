// Fill out your copyright notice in the Description page of Project Settings.


#include "ShaderTestBlueprintLibrary.h"
#include "GlobalShader.h"

#define LOCTEXT_NAMESPACE "ShaderTest"

UShaderTestBlueprintLibrary::UShaderTestBlueprintLibrary(const FObjectInitializer& ObjInitializer)
	: Super(ObjInitializer)
{
}

class FSimpleColorShader : public FGlobalShader
{
public:
	FSimpleColorShader()
	{
	}

	FSimpleColorShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		SimpleColorVal.Bind(Initializer.ParameterMap, TEXT("SimpleColor"));
	}

	static bool ShouldCache(EShaderPlatform Platform)
	{
		return true;
	}

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return true;
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters,
	                                         FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("TEST_MICRO"), 1);
	}

	void SetParameters(
		FRHICommandListImmediate& RHICmdList,
		const FLinearColor& Color
	)
	{
		// SetShaderValue(RHICmdList, GetPixelShader(), SimpleColorVal, Color);  
	}

private:
	FShaderParameter SimpleColorVal;
};

class FSimpleColorShaderVS : public FSimpleColorShader
{
	DECLARE_SHADER_TYPE(FSimpleColorShaderVS, Global);

public:
	FSimpleColorShaderVS()
	{
	}

	FSimpleColorShaderVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FSimpleColorShader(Initializer)
	{
	}
};

class FSimpleColorShaderPS : public FSimpleColorShader
{
	DECLARE_SHADER_TYPE(FSimpleColorShaderPS, Global);

public:
	FSimpleColorShaderPS()
	{
	}

	FSimpleColorShaderPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FSimpleColorShader(Initializer)
	{
	}
};

IMPLEMENT_SHADER_TYPE(, FSimpleColorShaderVS,
                        TEXT("/Plugins/UnrealEngineStudyLibrary/Shaders/Private/SimpleColorShader.usf"),
                        TEXT("MainVS"), SF_Vertex)

IMPLEMENT_SHADER_TYPE(, FSimpleColorShaderPS,
                        TEXT("/Plugins/UnrealEngineStudyLibrary/Shaders/Private/SimpleColorShader.usf"),
                        TEXT("MainPS"), SF_Pixel)

void UShaderTestBlueprintLibrary::DrawSimpleColorRenderTarget(
	UTextureRenderTarget2D* OutputRenderTarget, AActor* Actor, FLinearColor Color)
{
}

# undef LOCTEXT_NAMESPACE
