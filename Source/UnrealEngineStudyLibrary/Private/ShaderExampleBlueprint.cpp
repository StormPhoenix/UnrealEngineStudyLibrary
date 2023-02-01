// Fill out your copyright notice in the Description page of Project Settings.


#include "ShaderExampleBlueprint.h"
#include "GlobalShader.h"
#include "Engine/TextureRenderTarget2D.h"

#define LOCTEXT_NAMESPACE "ShaderExample"

UShaderExampleBlueprintLibrary::UShaderExampleBlueprintLibrary(const FObjectInitializer& ObjInitializer)
	: Super(ObjInitializer)
{
}

namespace ShaderExample
{
	// 自定义顶点数据
	struct FMyVertex
	{
		// 不能使用 FVector4 类型
		FVector4f Position;
		FVector2f UV;
	};

	class FMyVertexBuffer : public FVertexBuffer
	{
	public:
		void InitRHI() override
		{
			// create a static vertex buffer
			TResourceArray<FMyVertex, VERTEXBUFFER_ALIGNMENT> Vertices;
			Vertices.SetNumUninitialized(4);
			Vertices[0].Position = FVector4f(1, 1, 0, 1);
			Vertices[1].Position = FVector4f(-1, 1, 0, 1);
			Vertices[2].Position = FVector4f(1, -1, 0, 1);
			Vertices[3].Position = FVector4f(-1, -1, 0, 1);

			Vertices[0].UV = FVector2f(1.0f, 1.0f);
			Vertices[1].UV = FVector2f(0.0f, 1.0f);
			Vertices[2].UV = FVector2f(1.0f, 0.0f);
			Vertices[3].UV = FVector2f(0.0f, 0.0f);

			FRHIResourceCreateInfo CreateInfo(TEXT("FMyVertexBuffer"), &Vertices);
			VertexBufferRHI = RHICreateVertexBuffer(Vertices.GetResourceDataSize(), BUF_Static, CreateInfo);

			/*
			FRHIResourceCreateInfo CreateInfo(TEXT("FMyVertexBuffer"));
			VertexBufferRHI = RHICreateVertexBuffer(sizeof(FMyVertex) * 4, BUF_Static, CreateInfo);
			void* VoidPtr = RHILockBuffer(VertexBufferRHI, 0, sizeof(FMyVertex) * 4, RLM_WriteOnly);
			static FMyVertex Vertices[4];
			Vertices[0].Position = FVector4(1, 1, 0, 1);
			Vertices[1].Position = FVector4(-1, 1, 0, 1);
			Vertices[2].Position = FVector4(1, -1, 0, 1);
			Vertices[3].Position = FVector4(-1, -1, 0, 1);
			
			Vertices[0].UV = FVector2f(1.0f, 1.0f);
			Vertices[1].UV = FVector2f(0.0f, 1.0f);
			Vertices[2].UV = FVector2f(1.0f, 0.0f);
			Vertices[3].UV = FVector2f(0.0f, 0.0f);
			FMemory::Memcpy(VoidPtr, Vertices, sizeof(FMyVertex) * 4);
			RHIUnlockBuffer(VertexBufferRHI);
			*/
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
			IndexBufferRHI = RHICreateIndexBuffer(
				sizeof(uint16), IndexBuffer.GetResourceDataSize(), BUF_Static, CreateInfo);
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
			Elements.Add(FVertexElement(0, STRUCT_OFFSET(FMyVertex, UV), VET_Float2, 1, Stride));
			VertexDeclarationRHI = RHICreateVertexDeclaration(Elements);
		}

		virtual void ReleaseRHI() override
		{
			VertexDeclarationRHI.SafeRelease();
		}
	};

	TGlobalResource<FMyVertexBuffer> GMyVertexBuffer;
	TGlobalResource<FMyIndexBuffer> GMyIndexBuffer;

	class FSimpleExampleShader : public FGlobalShader
	{
		DECLARE_INLINE_TYPE_LAYOUT(FSimpleExampleShader, NonVirtual);

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

		FSimpleExampleShader()
		{
		}

		FSimpleExampleShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
			: FGlobalShader(Initializer)
		{
			MyColorVal.Bind(Initializer.ParameterMap, TEXT("MyColor"));
			MyTextureVal.Bind(Initializer.ParameterMap, TEXT("MyTexture"));
			MyTextureSamplerVal.Bind(Initializer.ParameterMap, TEXT("MyTextureSampler"));
		}

		template <typename TRHIShader>
		void SetParameters(FRHICommandListImmediate& RHICmdList, const TRHIShader ShaderRHI,
		                   const FLinearColor& InColor, FTexture2DRHIRef InTexture)
		{
			SetShaderValue(RHICmdList, ShaderRHI, MyColorVal, InColor);
			SetTextureParameter(RHICmdList, ShaderRHI, MyTextureVal, MyTextureSamplerVal,
			                    TStaticSamplerState<SF_Trilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI(), InTexture);
		}

	private:
		LAYOUT_FIELD(FShaderParameter, MyColorVal);
		LAYOUT_FIELD(FShaderResourceParameter, MyTextureVal);
		LAYOUT_FIELD(FShaderResourceParameter, MyTextureSamplerVal);
	};

	class FSimpleColorShaderVS : public FSimpleExampleShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FSimpleColorShaderVS);

		FSimpleColorShaderVS()
		{
		}

		FSimpleColorShaderVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
			FSimpleExampleShader(Initializer)
		{
		}
	};

	class FSimpleColorShaderPS : public FSimpleExampleShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FSimpleColorShaderPS);

		FSimpleColorShaderPS()
		{
		}

		FSimpleColorShaderPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
			FSimpleExampleShader(Initializer)
		{
		}
	};

	IMPLEMENT_GLOBAL_SHADER(
		FSimpleColorShaderVS, "/UnrealEngineStudyLibrary/Private/SimpleExampleShader.usf", "MainVS", SF_Vertex);

	IMPLEMENT_GLOBAL_SHADER(
		FSimpleColorShaderPS, "/UnrealEngineStudyLibrary/Private/SimpleExampleShader.usf", "MainPS", SF_Pixel);

	static void DrawSimpleColorRenderTarget_RenderThread(
		FRHICommandListImmediate& RHICmdList, FTextureRenderTargetResource* OutputRenderTargetResource,
		ERHIFeatureLevel::Type FeatureLevel, FLinearColor InColor, FTexture2DRHIRef InTexture)
	{
		check(IsInRenderingThread());

		FTexture2DRHIRef RenderTargetRHI = OutputRenderTargetResource->GetRenderTargetTexture();
		RHICmdList.Transition(FRHITransitionInfo(RenderTargetRHI, ERHIAccess::SRVMask, ERHIAccess::RTV));

		FRHIRenderPassInfo RenderPassInfo(RenderTargetRHI, ERenderTargetActions::DontLoad_Store);
		RHICmdList.BeginRenderPass(RenderPassInfo, TEXT("SimpleShaderExampleRenderPass"));
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

			RHICmdList.SetViewport(0, 0, 0, OutputRenderTargetResource->GetSizeX(),
			                       OutputRenderTargetResource->GetSizeY(), 1.0f);
			PixelShader->SetParameters(RHICmdList, PixelShader.GetPixelShader(), InColor, InTexture);
			RHICmdList.SetStreamSource(0, GMyVertexBuffer.VertexBufferRHI, 0);
			RHICmdList.DrawIndexedPrimitive(GMyIndexBuffer.IndexBufferRHI, 0, 0, 4, 0, 2, 1);
		}
		RHICmdList.EndRenderPass();
		RHICmdList.Transition(FRHITransitionInfo(RenderTargetRHI, ERHIAccess::RTV, ERHIAccess::SRVMask));
	}
}

void UShaderExampleBlueprintLibrary::DrawSimpleColorRenderTarget(
	const UObject* WorldContextObject, UTextureRenderTarget2D* OutputRenderTarget,
	FLinearColor InColor, UTexture2D* InTexture)
{
	check(IsInGameThread());

	if (OutputRenderTarget == nullptr)
	{
		return;
	}

	FTextureRenderTargetResource* TextureRenderTargetResource = OutputRenderTarget->
		GameThread_GetRenderTargetResource();
	FTexture2DRHIRef TextureRHI = InTexture->GetResource()->TextureRHI->GetTexture2D();

	const UWorld* World = WorldContextObject->GetWorld();
	ERHIFeatureLevel::Type FeatureLevel = World->Scene->GetFeatureLevel();
	FName TextureRenderTargetName = OutputRenderTarget->GetFName();
	ENQUEUE_RENDER_COMMAND(CaptureCommand)(
		[TextureRenderTargetResource, FeatureLevel, InColor, TextureRHI](FRHICommandListImmediate& RHICmdList)
		{
			ShaderExample::DrawSimpleColorRenderTarget_RenderThread(RHICmdList, TextureRenderTargetResource,
			                                                        FeatureLevel, InColor, TextureRHI);
		}
	);
}

#undef LOCTEXT_NAMESPACE
