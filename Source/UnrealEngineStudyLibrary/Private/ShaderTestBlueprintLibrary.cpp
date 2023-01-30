// Fill out your copyright notice in the Description page of Project Settings.


#include "ShaderTestBlueprintLibrary.h"
#include "GlobalShader.h"
#include "Engine/TextureRenderTarget2D.h"

#define LOCTEXT_NAMESPACE "ShaderTest"

UShaderTestBlueprintLibrary::UShaderTestBlueprintLibrary(const FObjectInitializer& ObjInitializer)
	: Super(ObjInitializer)
{
}

// 自定义顶点数据
struct FMyVertex
{
	// @todo 不能使用 FVector4 类型
	FVector4f Position;
};

class FMyVertexBuffer : public FVertexBuffer
{
public:
	void InitRHI() override
	{
		/*
		FRHIResourceCreateInfo CreateInfo(TEXT("FMyVertexBuffer"));
		VertexBufferRHI = RHICreateVertexBuffer(sizeof(FMyVertex) * 4, BUF_Static, CreateInfo);
		void* VoidPtr = RHILockBuffer(VertexBufferRHI, 0, sizeof(FMyVertex) * 4, RLM_WriteOnly);
		static FMyVertex Vertices[4];
		Vertices[0].Position = FVector4(1, 1, 0, 1);
		Vertices[1].Position = FVector4(-1, 1, 0, 1);
		Vertices[2].Position = FVector4(1, -1, 0, 1);
		Vertices[3].Position = FVector4(-1, -1, 0, 1);
		FMemory::Memcpy(VoidPtr, Vertices, sizeof(FMyVertex) * 4);
		RHIUnlockBuffer(VertexBufferRHI);
		*/

		// create a static vertex buffer
		TResourceArray<FMyVertex, VERTEXBUFFER_ALIGNMENT> Vertices;
		Vertices.SetNumUninitialized(4);
		Vertices[0].Position = FVector4f(1, 1, 0, 1);
		Vertices[1].Position = FVector4f(-1, 1, 0, 1);
		Vertices[2].Position = FVector4f(1, -1, 0, 1);
		Vertices[3].Position = FVector4f(-1, -1, 0, 1);

		FRHIResourceCreateInfo CreateInfo(TEXT("FMyVertexBuffer"), &Vertices);
		VertexBufferRHI = RHICreateVertexBuffer(Vertices.GetResourceDataSize(), BUF_Static, CreateInfo);
	}
};

class FMyIndexBuffer : public FIndexBuffer
{
public:
	void InitRHI() override
	{
		const uint16 Indices[] = {0, 1, 2, 2, 1, 3};
		TResourceArray<uint16, INDEXBUFFER_ALIGNMENT> IndexBuffer;
		uint32 NumIndices = UE_ARRAY_COUNT(Indices);
		IndexBuffer.AddUninitialized(NumIndices);
		FMemory::Memcpy(IndexBuffer.GetData(), Indices, NumIndices * sizeof(uint16));

		FRHIResourceCreateInfo CreateInfo(TEXT("FMyIndexBuffer"), &IndexBuffer);
		IndexBufferRHI = RHICreateIndexBuffer(sizeof(uint16), IndexBuffer.GetResourceDataSize(), BUF_Static,
		                                      CreateInfo);
	}
};

class FMyVertexDeclaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;

	virtual void InitRHI() override
	{
		FVertexDeclarationElementList Elements;
		uint32 Stride = sizeof(FMyVertex);
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FMyVertex, Position), VET_Float4, 0, Stride));
		// Elements.Add(FVertexElement(0, STRUCT_OFFSET(FMyVertex, Position), VET_Float4, 0, Stride));
		VertexDeclarationRHI = RHICreateVertexDeclaration(Elements);
	}

	virtual void ReleaseRHI() override
	{
		VertexDeclarationRHI.SafeRelease();
	}
};

TGlobalResource<FMyVertexBuffer> GMyVertexBuffer;
TGlobalResource<FMyIndexBuffer> GMyIndexBuffer;

class FSimpleColorShader : public FGlobalShader
{
	DECLARE_INLINE_TYPE_LAYOUT(FSimpleColorShader, NonVirtual);

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters,
	                                         FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("TEST_MICRO"), 1);
	}

	FSimpleColorShader()
	{
	}

	FSimpleColorShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		SimpleColorVal.Bind(Initializer.ParameterMap, TEXT("SimpleColor"));
	}

	// static bool ShouldCache(EShaderPlatform Platform)
	// {
	// 	return true;
	// }


	template <typename TRHIShader>
	void SetParameters(FRHICommandListImmediate& RHICmdList, const TRHIShader ShaderRHI, const FLinearColor& Color)
	{
		SetShaderValue(RHICmdList, ShaderRHI, SimpleColorVal, Color);
	}

private:
	LAYOUT_FIELD(FShaderParameter, SimpleColorVal);
};

class FSimpleColorShaderVS : public FSimpleColorShader
{
public:
	DECLARE_GLOBAL_SHADER(FSimpleColorShaderVS);

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
public:
	DECLARE_GLOBAL_SHADER(FSimpleColorShaderPS);

	FSimpleColorShaderPS()
	{
	}

	FSimpleColorShaderPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FSimpleColorShader(Initializer)
	{
	}
};

// IMPLEMENT_SHADER_TYPE(, FSimpleColorShaderVS,
//                         TEXT("/Plugins/UnrealEngineStudyLibrary/Private/SimpleColorShader.usf"),
//                         TEXT("MainVS"), SF_Vertex)
//
// IMPLEMENT_SHADER_TYPE(, FSimpleColorShaderPS,
//                         TEXT("/Plugins/UnrealEngineStudyLibrary/Private/SimpleColorShader.usf"),
//                         TEXT("MainPS"), SF_Pixel)

IMPLEMENT_GLOBAL_SHADER(FSimpleColorShaderVS,
                        "/UnrealEngineStudyLibrary/Private/SimpleColorShader.usf", "MainVS", SF_Vertex);

IMPLEMENT_GLOBAL_SHADER(FSimpleColorShaderPS,
                        "/UnrealEngineStudyLibrary/Private/SimpleColorShader.usf", "MainPS", SF_Pixel);

static void DrawSimpleColorRenderTarget_RenderThread(
	FRHICommandListImmediate& RHICmdList,
	FTextureRenderTargetResource* OutputRenderTargetResource,
	ERHIFeatureLevel::Type FeatureLevel,
	FLinearColor SimpleColor)
{
	check(IsInRenderingThread());

	// FRHIRenderPassInfo RenderPassInfo(RenderTargetRHI, ERenderTargetActions::DontLoad_Store,
	//                                   OutputRenderTargetResource->TextureRHI);

	// RHICmdList.TransitionResource(ERHIAccess::WritableMask, RenderTargetRHI);

	FTexture2DRHIRef RenderTargetRHI = OutputRenderTargetResource->GetRenderTargetTexture();
	RHICmdList.Transition(FRHITransitionInfo(RenderTargetRHI, ERHIAccess::SRVMask, ERHIAccess::RTV));

	FRHIRenderPassInfo RenderPassInfo(RenderTargetRHI, ERenderTargetActions::DontLoad_Store);
	RHICmdList.BeginRenderPass(RenderPassInfo, TEXT("TestSimpleColorRenderPass"));
	{
		// 获取着色器
		FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(FeatureLevel);
		TShaderMapRef<FSimpleColorShaderVS> VertexShader(GlobalShaderMap);
		TShaderMapRef<FSimpleColorShaderPS> PixelShader(GlobalShaderMap);

		FMyVertexDeclaration VertexDeclaration;
		VertexDeclaration.InitRHI();

		// 设置管线状态
		FGraphicsPipelineStateInitializer GraphicsPSOInit;
		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
		GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
		GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
		GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
		GraphicsPSOInit.PrimitiveType = PT_TriangleList;
		GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = VertexDeclaration.VertexDeclarationRHI;
		GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
		GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
		SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0);

		RHICmdList.SetViewport(0, 0, 0, OutputRenderTargetResource->GetSizeX(), OutputRenderTargetResource->GetSizeY(),
		                       1.0f);
		PixelShader->SetParameters(RHICmdList, PixelShader.GetPixelShader(), SimpleColor);
		RHICmdList.SetStreamSource(0, GMyVertexBuffer.VertexBufferRHI, 0);
		RHICmdList.DrawIndexedPrimitive(GMyIndexBuffer.IndexBufferRHI, 0, 0, 4, 0, 2, 1);
	}
	RHICmdList.EndRenderPass();
	RHICmdList.Transition(FRHITransitionInfo(RenderTargetRHI, ERHIAccess::RTV, ERHIAccess::SRVMask));
}

void UShaderTestBlueprintLibrary::DrawSimpleColorRenderTarget(
	const UObject* WorldContextObject, UTextureRenderTarget2D* OutputRenderTarget, AActor* Actor, FLinearColor Color)
{
	check(IsInGameThread());

	if (OutputRenderTarget == nullptr)
	{
		return;
	}

	FTextureRenderTargetResource* TextureRenderTargetResource = OutputRenderTarget->
		GameThread_GetRenderTargetResource();
	const UWorld* World = WorldContextObject->GetWorld();
	ERHIFeatureLevel::Type FeatureLevel = World->Scene->GetFeatureLevel();
	FName TextureRenderTargetName = OutputRenderTarget->GetFName();
	ENQUEUE_RENDER_COMMAND(CaptureCommand)(
		[TextureRenderTargetResource, FeatureLevel, Color](FRHICommandListImmediate& RHICmdList)
		{
			DrawSimpleColorRenderTarget_RenderThread(RHICmdList, TextureRenderTargetResource, FeatureLevel, Color);
		}
	);
}

# undef LOCTEXT_NAMESPACE
