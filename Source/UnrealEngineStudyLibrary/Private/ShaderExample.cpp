// Fill out your copyright notice in the Description page of Project Settings.


#include "ShaderExample.h"
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

			Vertices[0].UV = FVector2f(1.0f, 0.0f);
			Vertices[1].UV = FVector2f(0.0f, 0.0f);
			Vertices[2].UV = FVector2f(1.0f, 1.0f);
			Vertices[3].UV = FVector2f(0.0f, 1.0f);

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

	// Compute Shader
	class FMyComputerShader : public FGlobalShader
	{
		DECLARE_GLOBAL_SHADER(FMyComputerShader)
	public:
		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
		{
			return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
		}

		static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters,
		                                         FShaderCompilerEnvironment& OutEnvironment)
		{
		}

		FMyComputerShader()
		{
		}

		FMyComputerShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
			: FGlobalShader(Initializer)
		{
			OutSurface.Bind(Initializer.ParameterMap, TEXT("OutputSurface"));
			// OutSurface.Bind(Initializer.ParameterMap, TEXT("RWOutputSurface"));
		}

		void SetParameters(FRHICommandList& RHICmdList, FUnorderedAccessViewRHIRef& InUAV,
		                   FSimpleShaderParameter& ShaderParameter)
		{
			// @todo 为什么计算着色器的的 ShaderRHI 是由外界传进来的
			FRHIComputeShader* ShaderRHI = RHICmdList.GetBoundComputeShader();
			if (OutSurface.IsBound())
			{
				RHICmdList.SetUAVParameter(ShaderRHI, OutSurface.GetBaseIndex(), InUAV);
			}
			
			FSimpleUniformStructParameters SimpleUniformData;
			SimpleUniformData.Color1 = ShaderParameter.Color1;
			SimpleUniformData.Color2 = ShaderParameter.Color2;
			SimpleUniformData.Color3 = ShaderParameter.Color3;
			SimpleUniformData.Color4 = ShaderParameter.Color4;
			SimpleUniformData.ColorIndex = ShaderParameter.ColorIndex;
			SetUniformBufferParameterImmediate(
				RHICmdList, ShaderRHI, GetUniformBufferParameter<FSimpleUniformStructParameters>(),
				SimpleUniformData);
		}

		void UnbindBuffers(FRHICommandList& RHICmdList)
		{
			FRHIComputeShader* ShaderRHI = RHICmdList.GetBoundComputeShader();
			if (OutSurface.IsBound())
			{
				RHICmdList.SetUAVParameter(ShaderRHI, OutSurface.GetBaseIndex(), nullptr);
			}
		}

	private:
		LAYOUT_FIELD(FShaderResourceParameter, OutSurface);
	};

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
		                   const FLinearColor& InColor, FTexture2DRHIRef InTexture,
		                   FSimpleShaderParameter& ShaderParameter)
		{
			/* 参数 ShaderRHI 由外界传进来的 */
			// 设置 float 类型数据
			SetShaderValue(RHICmdList, ShaderRHI, MyColorVal, InColor);

			// 设置 texture 类型数据
			SetTextureParameter(RHICmdList, ShaderRHI, MyTextureVal, MyTextureSamplerVal,
			                    TStaticSamplerState<SF_Trilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI(), InTexture);

			FSimpleUniformStructParameters SimpleUniformData;
			SimpleUniformData.Color1 = ShaderParameter.Color1;
			SimpleUniformData.Color2 = ShaderParameter.Color2;
			SimpleUniformData.Color3 = ShaderParameter.Color3;
			SimpleUniformData.Color4 = ShaderParameter.Color4;
			SimpleUniformData.ColorIndex = ShaderParameter.ColorIndex;
			SetUniformBufferParameterImmediate(
				RHICmdList, ShaderRHI, GetUniformBufferParameter<FSimpleUniformStructParameters>(),
				SimpleUniformData);
		}

	private:
		LAYOUT_FIELD(FShaderParameter, MyColorVal);
		LAYOUT_FIELD(FShaderResourceParameter, MyTextureVal);
		LAYOUT_FIELD(FShaderResourceParameter, MyTextureSamplerVal);
	};

	class FSimpleExampleShaderVS : public FSimpleExampleShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FSimpleExampleShaderVS);

		FSimpleExampleShaderVS()
		{
		}

		FSimpleExampleShaderVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
			FSimpleExampleShader(Initializer)
		{
		}
	};

	class FSimpleExampleShaderPS : public FSimpleExampleShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FSimpleExampleShaderPS);

		FSimpleExampleShaderPS()
		{
		}

		FSimpleExampleShaderPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
			FSimpleExampleShader(Initializer)
		{
		}
	};

	// Implement uniforms
	IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FSimpleUniformStructParameters, "SimpleUniformStruct");

	// Implement shaders
	IMPLEMENT_SHADER_TYPE(,
		FMyComputerShader, TEXT("/UnrealEngineStudyLibrary/Private/ComputeShaderExample.usf"), TEXT("MainCS"),
		SF_Compute);

	// IMPLEMENT_GLOBAL_SHADER(FMyComputerShader, "/UnrealEngineStudyLibrary/Private/ComputeShaderExample.usf", "MainCS",
	//                         SF_Compute);

	IMPLEMENT_GLOBAL_SHADER(
		FSimpleExampleShaderVS, "/UnrealEngineStudyLibrary/Private/SimpleExampleShader.usf", "MainVS", SF_Vertex);

	IMPLEMENT_GLOBAL_SHADER(
		FSimpleExampleShaderPS, "/UnrealEngineStudyLibrary/Private/SimpleExampleShader.usf", "MainPS", SF_Pixel);

	static void DrawSimpleColorRenderTarget_RenderThread(
		FRHICommandListImmediate& RHICmdList, FTextureRenderTargetResource* OutputRenderTargetResource,
		ERHIFeatureLevel::Type FeatureLevel, FLinearColor InColor, FTexture2DRHIRef InTexture,
		FSimpleShaderParameter ShaderStructData)
	{
		check(IsInRenderingThread());

		FTexture2DRHIRef RenderTargetRHI = OutputRenderTargetResource->GetRenderTargetTexture();
		RHICmdList.Transition(FRHITransitionInfo(RenderTargetRHI, ERHIAccess::SRVMask, ERHIAccess::RTV));

		FRHIRenderPassInfo RenderPassInfo(RenderTargetRHI, ERenderTargetActions::DontLoad_Store);
		RHICmdList.BeginRenderPass(RenderPassInfo, TEXT("SimpleShaderExampleRenderPass"));
		{
			// 获取着色器
			FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(FeatureLevel);
			TShaderMapRef<FSimpleExampleShaderVS> VertexShader(GlobalShaderMap);
			TShaderMapRef<FSimpleExampleShaderPS> PixelShader(GlobalShaderMap);

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
			PixelShader->SetParameters(RHICmdList, PixelShader.GetPixelShader(), InColor, InTexture, ShaderStructData);
			RHICmdList.SetStreamSource(0, GMyVertexBuffer.VertexBufferRHI, 0);
			RHICmdList.DrawIndexedPrimitive(GMyIndexBuffer.IndexBufferRHI, 0, 0, 4, 0, 2, 1);
		}
		RHICmdList.EndRenderPass();
		RHICmdList.Transition(FRHITransitionInfo(RenderTargetRHI, ERHIAccess::RTV, ERHIAccess::SRVMask));
	}

	static void ComputeShaderDraw_RenderThread(
		FRHICommandListImmediate& RHICmdList, FTextureRenderTargetResource* OutputRenderTargetResource,
		ERHIFeatureLevel::Type FeatureLevel, FLinearColor InColor, FSimpleShaderParameter ShaderStructData)
	{
		check(IsInRenderingThread());

		// 初始化 Compute Shader 内线程块数量
		uint32 GGroupSize = 32;
		FTexture2DRHIRef RenderTargetRHI = OutputRenderTargetResource->GetRenderTargetTexture();
		FIntPoint FullResolution = FIntPoint(RenderTargetRHI->GetSizeX(), RenderTargetRHI->GetSizeY());
		uint32 GroupSizeX = FMath::DivideAndRoundUp((uint32) RenderTargetRHI->GetSizeX(), GGroupSize);
		uint32 GroupSizeY = FMath::DivideAndRoundUp((uint32) RenderTargetRHI->GetSizeY(), GGroupSize);

		// 绑定 ComputeShader 到当前的渲染状态中
		FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(FeatureLevel);
		TShaderMapRef<FMyComputerShader> ComputeShader(GlobalShaderMap);
		RHICmdList.SetComputeShader(ComputeShader.GetComputeShader());

		// 创建临时纹理，用于 ComputeShader 做绘制。该绘制结果最终将存储到临时纹理中.
		FRHIResourceCreateInfo CreateInfo(TEXT("ShaderExample_ComputeShader_UAV"));
		FTexture2DRHIRef OutSurfaceTexture = RHICreateTexture2D(RenderTargetRHI->GetSizeX(), RenderTargetRHI->GetSizeX(), PF_A32B32G32R32F, 1, 1, TexCreate_ShaderResource | TexCreate_UAV, CreateInfo);
		// 将临时纹理封装成 UAV
		FUnorderedAccessViewRHIRef OutSurfaceTextureUAV = RHICreateUnorderedAccessView(OutSurfaceTexture);

		// 绑定 UAV 到 Compute Shader
		ComputeShader->SetParameters(RHICmdList, OutSurfaceTextureUAV, ShaderStructData);
		// Compute Shader 开始执行计算
		DispatchComputeShader(RHICmdList, ComputeShader, GroupSizeX, GroupSizeY, 1);
		// 解绑 UAV
		ComputeShader->UnbindBuffers(RHICmdList);

		// FRHICopyTextureInfo CopyInfo;
		// RHICmdList.CopyTexture(OutSurfaceTexture, RenderTargetRHI, CopyInfo);

		// 将临时纹理作为参数，后续步骤将进行采样
		DrawSimpleColorRenderTarget_RenderThread(
			RHICmdList, OutputRenderTargetResource, FeatureLevel, InColor, OutSurfaceTexture, ShaderStructData);
	}
}

void UShaderExampleBlueprintLibrary::DrawSimpleColorRenderTarget(
	const UObject* WorldContextObject, UTextureRenderTarget2D* OutputRenderTarget,
	FLinearColor InColor, UTexture2D* InTexture, FSimpleShaderParameter ShaderStructData)
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
		[TextureRenderTargetResource, FeatureLevel, InColor, TextureRHI,
			ShaderStructData](FRHICommandListImmediate& RHICmdList)
		{
			ShaderExample::DrawSimpleColorRenderTarget_RenderThread(
				RHICmdList, TextureRenderTargetResource, FeatureLevel, InColor, TextureRHI, ShaderStructData);
		}
	);
}

void UShaderExampleBlueprintLibrary::ComputeShaderDraw(const UObject* WorldContext,
	UTextureRenderTarget2D* OutputRenderTarget, FLinearColor InColor, FSimpleShaderParameter ShaderStructData)
{
	check(IsInGameThread());

	if (OutputRenderTarget == nullptr)
	{
		return;
	}

	FTextureRenderTargetResource* TextureRenderTargetResource = OutputRenderTarget->
		GameThread_GetRenderTargetResource();

	const UWorld* World = WorldContext->GetWorld();
	ERHIFeatureLevel::Type FeatureLevel = World->Scene->GetFeatureLevel();
	FName TextureRenderTargetName = OutputRenderTarget->GetFName();
	ENQUEUE_RENDER_COMMAND(CaptureCommand)(
		[TextureRenderTargetResource, FeatureLevel, InColor,
			ShaderStructData](FRHICommandListImmediate& RHICmdList)
		{
			ShaderExample::ComputeShaderDraw_RenderThread(
				RHICmdList, TextureRenderTargetResource, FeatureLevel, InColor , ShaderStructData);
		}
	);
	
}


#undef LOCTEXT_NAMESPACE
