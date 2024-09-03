#include "GFX/Pipeline/CoreRenderer.h"
#include "GFX/Pipeline/RenderPasses.h"

namespace ZE::GFX::Pipeline::CoreRenderer
{
	void SetupRenderSlots(RenderGraphDesc& graphDesc) noexcept
	{
		constexpr Resource::Texture::AddressMode ADDRESS_MODES[]
		{
			Resource::Texture::AddressMode::Edge,
			Resource::Texture::AddressMode::Repeat,
			Resource::Texture::AddressMode::Mirror
		};
		constexpr Resource::SamplerType TYPES[]
		{
			Resource::SamplerType::Anisotropic,
			Resource::SamplerType::Linear,
			Resource::SamplerType::Point
		};

		graphDesc.Samplers.reserve(sizeof(TYPES) * sizeof(ADDRESS_MODES));
		U32 slot = 0;
		for (U8 type = 0; type < sizeof(TYPES); ++type)
		{
			for (U8 address = 0; address < sizeof(ADDRESS_MODES); ++address)
			{
				graphDesc.Samplers.emplace_back(Resource::SamplerDesc
					{
						TYPES[type],
						{
							ADDRESS_MODES[address],
							ADDRESS_MODES[address],
							ADDRESS_MODES[address]
						},
						0.0f, 4, Resource::CompareMethod::Never,
						Resource::Texture::EdgeColor::TransparentBlack,
						0.0f, FLT_MAX, slot++
					});
			}
		}

		graphDesc.SettingsRange =
		{
			1, 13, 0,
			Resource::ShaderType::All,
			Binding::RangeFlag::CBV | Binding::RangeFlag::GlobalBuffer
		};
		graphDesc.DynamicDataRange =
		{
			1, 12, 1,
			Resource::ShaderType::All,
			Binding::RangeFlag::CBV | Binding::RangeFlag::GlobalBuffer
		};
	}

	void SetupData(RendererSettingsData& settingsData, const Params& params, UInt2 outlineBuffSize) noexcept
	{
		settingsData.DisplaySize = Settings::DisplaySize;
		settingsData.RenderSize = Settings::RenderSize;

		settingsData.AmbientLight = { 0.03f, 0.03f, 0.03f };
		settingsData.HDRExposure = params.HDRExposure;

		settingsData.BlurWidth = outlineBuffSize.X;
		settingsData.BlurHeight = outlineBuffSize.Y;
		settingsData.BlurRadius = RendererSettingsData::BLUR_KERNEL_RADIUS;
		settingsData.BlurIntensity = 1.0f;
		if (settingsData.HDRExposure < 1.0f)
			settingsData.BlurIntensity /= settingsData.HDRExposure;

		settingsData.BlurSigma = params.Sigma;
		settingsData.ShadowMapSize = Utils::SafeCast<float>(params.ShadowMapSize);
		settingsData.ShadowBias = Utils::SafeCast<float>(params.ShadowBias) / settingsData.ShadowMapSize;
		settingsData.ShadowNormalOffset = params.ShadowNormalOffset;

		settingsData.MipBias = CalculateMipBias(Settings::RenderSize.X, Settings::DisplaySize.X, Settings::GetUpscaler());
		settingsData.Gamma = params.Gamma;
		settingsData.GammaInverse = 1.0f / params.Gamma;
		settingsData.ReactiveMaskClamp = GetReactiveMaskClamp(Settings::GetUpscaler());

		// Setup blur kernel
		float gaussSum = 0.0f;
		for (S32 i = 0; i <= settingsData.BlurRadius; ++i)
		{
			const float g = Math::Gauss(Utils::SafeCast<float>(i), settingsData.BlurSigma);
			gaussSum += g;
			settingsData.BlurCoefficients[i].x = g;
		}
		for (S32 i = 0; i <= settingsData.BlurRadius; ++i)
			settingsData.BlurCoefficients[i].x /= gaussSum;
	}

	RenderGraphDesc GetDesc(const Params& params) noexcept
	{
		constexpr UInt2 SIZE_SYNC = { 1, 1 };
		constexpr UInt2 OUTLINE_SIZE_SCALING = { 2, 2 };
		RenderGraphDesc graphDesc;

		SetupRenderSlots(graphDesc);
		SetupData(graphDesc.SettingsData, params,
			{ Settings::DisplaySize.X / OUTLINE_SIZE_SCALING.X, Settings::DisplaySize.Y / OUTLINE_SIZE_SCALING.Y });
		graphDesc.InitBuffers(16);

#pragma region Framebuffer definition
		// GBuffer related resources
		graphDesc.AddResource("gbuffDepth",
			{ SIZE_SYNC, 1, FrameResourceFlag::SyncRenderSize | FrameResourceFlag::ForceSRV, PixelFormat::DepthOnly, ColorF4(), 0.0f, 0, 1 ZE_FRAME_RES_INIT_NAME("GBuff depth") });
		graphDesc.AddResource("gbuffNormal",
			{ SIZE_SYNC, 1, static_cast<FrameResourceFlags>(FrameResourceFlag::SyncRenderSize), PixelFormat::R16G16_Float, ColorF4(), 0.0f, 0, 1 ZE_FRAME_RES_INIT_NAME("GBuff normals") });
		graphDesc.AddResource("gbuffAlbedo",
			{ SIZE_SYNC, 1, static_cast<FrameResourceFlags>(FrameResourceFlag::SyncRenderSize), PixelFormat::R16G16B16A16_UNorm, ColorF4(), 0.0f, 0, 1 ZE_FRAME_RES_INIT_NAME("GBuff albedo") });
		graphDesc.AddResource("gbuffMaterial",
			{ SIZE_SYNC, 1, static_cast<FrameResourceFlags>(FrameResourceFlag::SyncRenderSize), PixelFormat::R8G8_UNorm, ColorF4(), 0.0f, 0, 1 ZE_FRAME_RES_INIT_NAME("GBuff material params") }); // R - metalness, G - roughness
		graphDesc.AddResource("gbuffMotion",
			{ SIZE_SYNC, 1, static_cast<FrameResourceFlags>(FrameResourceFlag::SyncRenderSize), PixelFormat::R16G16_Float, ColorF4(), 0.0f, 0, 1 ZE_FRAME_RES_INIT_NAME("GBuff motion vectors") });
		graphDesc.AddResource("gbuffReactive",
			{ SIZE_SYNC, 1, static_cast<FrameResourceFlags>(FrameResourceFlag::SyncRenderSize), PixelFormat::R8_UNorm, ColorF4(), 0.0f, 0, 1 ZE_FRAME_RES_INIT_NAME("GBuff reactive mask") });

		// Copy of resources for async compute SSAO
		graphDesc.AddResource("gbuffDepthCompute",
			{ SIZE_SYNC, 1, FrameResourceFlag::SyncRenderSize | FrameResourceFlag::ForceDSV, PixelFormat::DepthOnly, ColorF4(), 0.0f, 0, 1 ZE_FRAME_RES_INIT_NAME("GBuff depth compute copy") });
		graphDesc.AddResource("gbuffNormalCompute",
			{ SIZE_SYNC, 1, static_cast<FrameResourceFlags>(FrameResourceFlag::SyncRenderSize), PixelFormat::R16G16_Float, ColorF4(), 0.0f, 0, 1 ZE_FRAME_RES_INIT_NAME("GBuff normals compute copy") });

		// Light related resources
		graphDesc.AddResource("directLighting",
			{ SIZE_SYNC, 1, static_cast<FrameResourceFlags>(FrameResourceFlag::SyncRenderSize), PixelFormat::R16G16B16A16_Float, ColorF4(0.0f, 0.0f, 0.0f, 0.0f), 0.0f, 0, 1 ZE_FRAME_RES_INIT_NAME("Direct lighting") });
		graphDesc.AddResource("ssao",
			{ SIZE_SYNC, 1, FrameResourceFlag::SyncRenderSize | FrameResourceFlag::ForceSRV, PixelFormat::R8_UInt, ColorF4(), 0.0f, 0, 1 ZE_FRAME_RES_INIT_NAME("SSAO") });
		if (false)
			graphDesc.AddResource("ssr",
				{ SIZE_SYNC, 1, static_cast<FrameResourceFlags>(FrameResourceFlag::SyncRenderSize), PixelFormat::R16G16B16A16_Float, ColorF4(), 0.0f, 0, 1 ZE_FRAME_RES_INIT_NAME("SSR") });

		// Combined scene related resources
		graphDesc.AddResource("rawScene",
			{ SIZE_SYNC, 1, static_cast<FrameResourceFlags>(FrameResourceFlag::SyncRenderSize), PixelFormat::R16G16B16A16_Float, ColorF4(), 0.0f, 0, 1 ZE_FRAME_RES_INIT_NAME("Raw lighted scene") });
		graphDesc.AddResource("upscaledScene",
			{ SIZE_SYNC, 1, static_cast<FrameResourceFlags>(FrameResourceFlag::SyncDisplaySize), PixelFormat::R16G16B16A16_Float, ColorF4(), 0.0f, 0, 1 ZE_FRAME_RES_INIT_NAME("Upscaled scene") });

		// Outline related resources
		graphDesc.AddResource("outlineDepth",
			{ SIZE_SYNC, 1, static_cast<FrameResourceFlags>(FrameResourceFlag::SyncDisplaySize), PixelFormat::DepthStencil, ColorF4(), 0.0f, 0, 1 ZE_FRAME_RES_INIT_NAME("Outline depth") });
		graphDesc.AddResource("outline",
			{ OUTLINE_SIZE_SCALING, 1, FrameResourceFlag::SyncDisplaySize | FrameResourceFlag::SyncScalingDivide, Settings::BackbufferFormat, ColorF4(0.0f, 0.0f, 0.0f, 0.0f), 0.0f, 0, 1 ZE_FRAME_RES_INIT_NAME("Outline") });
		graphDesc.AddResource("outlineBlur",
			{ OUTLINE_SIZE_SCALING, 1, FrameResourceFlag::SyncDisplaySize | FrameResourceFlag::SyncScalingDivide, Settings::BackbufferFormat, ColorF4(0.0f, 0.0f, 0.0f, 0.0f), 0.0f, 0, 1 ZE_FRAME_RES_INIT_NAME("Outline blur") });
#pragma endregion

		// TODO: Don't make passes dependent on input RID during creation,
		// let them decide during execution and by setting correct output flags as required=false
		// Also add ALL possible passes to the list, even the ones that won't be run and only decide for them during creation with evaluate function
		//
		// For future, before stepping into this function, maybe at settings level, will need to read configuration for correct upscaling quality.
		// This will be kept in render passes configuration area so for given current

#pragma region Geometry
		{
			RenderPass::ClearBuffer<4>::ExecuteData clearInfo;
			clearInfo.Info[0].BufferType = RenderPass::ClearBufferType::DSV;
			clearInfo.Info[0].ClearValue.DSV.Depth = 0.0f;
			clearInfo.Info[0].ClearValue.DSV.Stencil = 0;
			clearInfo.Info[1].BufferType = RenderPass::ClearBufferType::RTV;
			clearInfo.Info[1].ClearValue.Color = ColorF4();
			clearInfo.Info[2].BufferType = RenderPass::ClearBufferType::RTV;
			clearInfo.Info[2].ClearValue.Color = ColorF4();
			clearInfo.Info[3].BufferType = RenderPass::ClearBufferType::RTV;
			clearInfo.Info[3].ClearValue.Color = ColorF4();

			RenderNode node("gbufferClear", "", RenderPass::ClearBuffer<4>::GetDesc(static_cast<PassType>(CorePassType::GBufferClear),
				clearInfo), PassExecutionType::Producer);
			node.AddOutput("DS", TextureLayout::DepthStencilWrite, "gbuffDepth");
			node.AddOutput("GB_N", TextureLayout::RenderTarget, "gbuffNormal");
			node.AddOutput("GB_ALB", TextureLayout::RenderTarget, "gbuffAlbedo");
			node.AddOutput("GB_MAT", TextureLayout::RenderTarget, "gbuffMaterial");
			graphDesc.RenderPasses.emplace_back(std::move(node));
		}
		{
			RenderPass::ClearBuffer<1>::ExecuteData clearInfo;
			clearInfo.Info[0].BufferType = RenderPass::ClearBufferType::RTV;
			clearInfo.Info[0].ClearValue.Color = ColorF4();

			RenderNode node("motionClear", "", RenderPass::ClearBuffer<1>::GetDesc(static_cast<PassType>(CorePassType::MotionVectorsClear),
				clearInfo, []() noexcept { return Settings::ComputeMotionVectors(); }), PassExecutionType::Producer);
			node.AddOutput("GB_MV", TextureLayout::RenderTarget, "gbuffMotion");
			graphDesc.RenderPasses.emplace_back(std::move(node));
		}
		{
			RenderPass::ClearBuffer<1>::ExecuteData clearInfo;
			clearInfo.Info[0].BufferType = RenderPass::ClearBufferType::RTV;
			clearInfo.Info[0].ClearValue.Color = ColorF4();

			RenderNode node("reactiveClear", "", RenderPass::ClearBuffer<1>::GetDesc(static_cast<PassType>(CorePassType::ReactiveMaskClear),
				clearInfo, []() noexcept { return Settings::GetUpscaler() == UpscalerType::Fsr2 || Settings::GetUpscaler() == UpscalerType::XeSS; }), PassExecutionType::Producer);
			node.AddOutput("GB_R", TextureLayout::RenderTarget, "gbuffReactive");
			graphDesc.RenderPasses.emplace_back(std::move(node));
		}
		{
			RenderNode node("lambertian", "", RenderPass::Lambertian::GetDesc(graphDesc.GetFormat("gbuffDepth"),
				graphDesc.GetFormat("gbuffNormal"), graphDesc.GetFormat("gbuffAlbedo"), graphDesc.GetFormat("gbuffMaterial"),
				graphDesc.GetFormat("gbuffMotion"), graphDesc.GetFormat("gbuffReactive")), PassExecutionType::DynamicProcessor);
			node.AddInput("gbufferClear.DS", TextureLayout::DepthStencilWrite);
			node.AddInput("gbufferClear.GB_N", TextureLayout::RenderTarget);
			node.AddInput("gbufferClear.GB_ALB", TextureLayout::RenderTarget);
			node.AddInput("gbufferClear.GB_MAT", TextureLayout::RenderTarget);
			node.AddInput("motionClear.GB_MV", TextureLayout::RenderTarget, false);
			node.AddInput("reactiveClear.GB_R", TextureLayout::RenderTarget, false);
			node.AddOutput("DS", TextureLayout::DepthStencilWrite, "gbuffDepth");
			node.AddOutput("GB_N", TextureLayout::RenderTarget, "gbuffNormal");
			node.AddOutput("GB_ALB", TextureLayout::RenderTarget, "gbuffAlbedo");
			node.AddOutput("GB_MAT", TextureLayout::RenderTarget, "gbuffMaterial");
			node.AddOutput("GB_MV", TextureLayout::RenderTarget, "gbuffMotion");
			node.AddOutput("GB_R", TextureLayout::RenderTarget, "gbuffReactive");
			node.SetHintGfx();
			graphDesc.RenderPasses.emplace_back(std::move(node));
		}
		{
			RenderNode node("lambertianComputeCopy", "", RenderPass::LambertianComputeCopy::GetDesc(), PassExecutionType::Processor);
			node.AddInput("lambertian.DS", TextureLayout::CopySource);
			node.AddInput("lambertian.GB_N", TextureLayout::CopySource);
			node.AddOutput("DS", TextureLayout::CopyDest, "gbuffDepthCompute", "gbuffDepth");
			node.AddOutput("GB_N", TextureLayout::CopyDest, "gbuffNormalCompute", "gbuffNormal");
			graphDesc.RenderPasses.emplace_back(std::move(node));
		}
#pragma endregion
#pragma region Lightning
		{
			RenderPass::ClearBuffer<1>::ExecuteData clearInfo;
			clearInfo.Info[0].BufferType = RenderPass::ClearBufferType::RTV;
			clearInfo.Info[0].ClearValue.Color = ColorF4(0.0f, 0.0f, 0.0f, 0.0f);

			RenderNode node("lightClear", "", RenderPass::ClearBuffer<1>::GetDesc(static_cast<PassType>(CorePassType::LightClear),
				clearInfo), PassExecutionType::Producer);
			node.AddOutput("LB", TextureLayout::RenderTarget, "directLighting");
			graphDesc.RenderPasses.emplace_back(std::move(node));
		}
		{
			RenderNode node("dirLight", "", RenderPass::DirectionalLight::GetDesc(graphDesc.GetFormat("directLighting"),
				PixelFormat::R32_Float, PixelFormat::DepthOnly), PassExecutionType::DynamicProcessor);
			node.AddInput("lambertian.DS", TextureLayout::ShaderResource);
			node.AddInput("lambertian.GB_N", TextureLayout::ShaderResource);
			node.AddInput("lambertian.GB_ALB", TextureLayout::ShaderResource);
			node.AddInput("lambertian.GB_MAT", TextureLayout::ShaderResource);
			node.AddInput("lightClear.LB", TextureLayout::RenderTarget);
			node.AddInnerBuffer(TextureLayout::RenderTarget,
				{ params.ShadowMapSize, params.ShadowMapSize, 1, static_cast<FrameResourceFlags>(FrameResourceFlag::ForceSRV), PixelFormat::R32_Float, { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX }, 0.0f, 0, 1 ZE_FRAME_RES_INIT_NAME("Direct light shadow map") });
			node.AddInnerBuffer(TextureLayout::DepthStencilWrite,
				{ params.ShadowMapSize, params.ShadowMapSize, 1, static_cast<FrameResourceFlags>(FrameResourceFlag::None), PixelFormat::DepthOnly, ColorF4(), 0.0f, 0, 1 ZE_FRAME_RES_INIT_NAME("Direct light shadow map depth") });
			node.AddOutput("LB", TextureLayout::RenderTarget, "directLighting");
			node.SetHintGfx();
			graphDesc.RenderPasses.emplace_back(std::move(node));
		}
		{
			RenderNode node("spotLight", "", RenderPass::SpotLight::GetDesc(graphDesc.GetFormat("directLighting"),
				PixelFormat::R32_Float, PixelFormat::DepthOnly), PassExecutionType::DynamicProcessor);
			node.AddInput("lambertian.DS", TextureLayout::ShaderResource);
			node.AddInput("lambertian.GB_N", TextureLayout::ShaderResource);
			node.AddInput("lambertian.GB_ALB", TextureLayout::ShaderResource);
			node.AddInput("lambertian.GB_MAT", TextureLayout::ShaderResource);
			node.AddInput("dirLight.LB", TextureLayout::RenderTarget);
			node.AddInnerBuffer(TextureLayout::RenderTarget,
				{ params.ShadowMapSize, params.ShadowMapSize, 1, static_cast<FrameResourceFlags>(FrameResourceFlag::ForceSRV), PixelFormat::R32_Float, { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX }, 0.0f, 0, 1 ZE_FRAME_RES_INIT_NAME("Spot light shadow map") });
			node.AddInnerBuffer(TextureLayout::DepthStencilWrite,
				{ params.ShadowMapSize, params.ShadowMapSize, 1, static_cast<FrameResourceFlags>(FrameResourceFlag::None), PixelFormat::DepthOnly, ColorF4(), 0.0f, 0, 1 ZE_FRAME_RES_INIT_NAME("Spot light shadow map depth") });
			node.AddOutput("LB", TextureLayout::RenderTarget, "directLighting");
			node.SetHintGfx();
			graphDesc.RenderPasses.emplace_back(std::move(node));
		}
		{
			RenderNode node("pointLight", "", RenderPass::PointLight::GetDesc(graphDesc.GetFormat("directLighting"),
				PixelFormat::R32_Float, PixelFormat::DepthOnly), PassExecutionType::DynamicProcessor);
			node.AddInput("lambertian.DS", TextureLayout::ShaderResource);
			node.AddInput("lambertian.GB_N", TextureLayout::ShaderResource);
			node.AddInput("lambertian.GB_ALB", TextureLayout::ShaderResource);
			node.AddInput("lambertian.GB_MAT", TextureLayout::ShaderResource);
			node.AddInput("spotLight.LB", TextureLayout::RenderTarget);
			node.AddInnerBuffer(TextureLayout::RenderTarget,
				{ params.ShadowMapSize, params.ShadowMapSize, 1, FrameResourceFlag::Cube | FrameResourceFlag::ForceSRV, PixelFormat::R32_Float, { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX }, 0.0f, 0, 1 ZE_FRAME_RES_INIT_NAME("Point light shadow map") });
			node.AddInnerBuffer(TextureLayout::DepthStencilWrite,
				{ params.ShadowMapSize, params.ShadowMapSize, 1, static_cast<FrameResourceFlags>(FrameResourceFlag::Cube), PixelFormat::DepthOnly, ColorF4(), 0.0f, 0, 1 ZE_FRAME_RES_INIT_NAME("Point light shadow map depth") });
			node.AddOutput("LB", TextureLayout::RenderTarget, "directLighting");
			node.SetHintGfx();
			graphDesc.RenderPasses.emplace_back(std::move(node));
		}
		{
			RenderNode node("ssao", "xegtao", RenderPass::XeGTAO::GetDesc(), PassExecutionType::Producer, Settings::IsEnabledAsyncAO());
			node.AddInput("lambertianComputeCopy.DS", TextureLayout::ShaderResource);
			node.AddInput("lambertianComputeCopy.GB_N", TextureLayout::ShaderResource);
			node.AddInnerBuffer(TextureLayout::UnorderedAccess,
				{ SIZE_SYNC, 1, FrameResourceFlag::SyncRenderSize | FrameResourceFlag::ForceSRV, PixelFormat::R32_Float, ColorF4(), 0.0f, 0, 5 ZE_FRAME_RES_INIT_NAME("XeGTAO viewspace depth") });
			node.AddInnerBuffer(TextureLayout::UnorderedAccess,
				{ SIZE_SYNC, 1, FrameResourceFlag::SyncRenderSize | FrameResourceFlag::ForceSRV, graphDesc.GetFormat("ssao"), ColorF4(), 0.0f, 0, 1 ZE_FRAME_RES_INIT_NAME("XeGTAO scratch AO") });
			node.AddInnerBuffer(TextureLayout::UnorderedAccess,
				{ SIZE_SYNC, 1, FrameResourceFlag::SyncRenderSize | FrameResourceFlag::ForceSRV, PixelFormat::R8_UNorm, ColorF4(), 0.0f, 0, 1 ZE_FRAME_RES_INIT_NAME("XeGTAO depth edges") });
			node.AddOutput("SB", TextureLayout::UnorderedAccess, "ssao");
			node.SetHintCompute();
			graphDesc.RenderPasses.emplace_back(std::move(node));
		}
		{
			RenderNode node("ssao", "cacao", RenderPass::CACAO::GetDesc(), PassExecutionType::Producer, Settings::IsEnabledAsyncAO());
			node.AddInput("lambertianComputeCopy.DS", TextureLayout::ShaderResource);
			node.AddInput("lambertianComputeCopy.GB_N", TextureLayout::ShaderResource);
			node.AddOutput("SB", TextureLayout::UnorderedAccess, "ssao");
			node.SetHintCompute();
			graphDesc.RenderPasses.emplace_back(std::move(node));
		}
		{
			RenderNode node("lightCombine", "", RenderPass::LightCombine::GetDesc(graphDesc.GetFormat("rawScene")), PassExecutionType::Producer);
			node.AddInput("pointLight.LB", TextureLayout::ShaderResource);
			node.AddInput("ssao.SB", TextureLayout::ShaderResource, false);
			node.AddOutput("RT", TextureLayout::RenderTarget, "rawScene");
			node.SetHintGfx();
			graphDesc.RenderPasses.emplace_back(std::move(node));
		}
#pragma endregion
#pragma region Post process render size
		{
			RenderNode node("skybox", "", RenderPass::Skybox::GetDesc(graphDesc.GetFormat("rawScene"),
				graphDesc.GetFormat("gbuffDepth"), params.SkyboxPath, params.SkyboxExt), PassExecutionType::Processor);
			node.AddInput("lightCombine.RT", TextureLayout::RenderTarget);
			node.AddInput("lambertian.DS", TextureLayout::DepthStencilRead);
			node.AddOutput("RT", TextureLayout::RenderTarget, "rawScene");
			node.AddOutput("DS", TextureLayout::DepthStencilRead, "gbuffDepth");
			node.SetHintGfx();
			graphDesc.RenderPasses.emplace_back(std::move(node));
		}
		{
			RenderNode node("wireframe", "", RenderPass::Wireframe::GetDesc(graphDesc.GetFormat("rawScene"),
				graphDesc.GetFormat("gbuffDepth")), PassExecutionType::DynamicProcessor);
			node.AddInput("skybox.RT", TextureLayout::RenderTarget);
			node.AddInput("skybox.DS", TextureLayout::DepthStencilWrite);
			node.AddOutput("RT", TextureLayout::RenderTarget, "rawScene");
			node.AddOutput("DS", TextureLayout::DepthStencilWrite, "gbuffDepth");
			node.SetHintGfx();
			graphDesc.RenderPasses.emplace_back(std::move(node));
		}
		if (false)
		{
			RenderNode node("ssr", "sssr", RenderPass::SSSR::GetDesc(), PassExecutionType::Producer);
			//node.AddInput("wireframe.RT", TextureLayout::ShaderResource); ??
			node.AddInput("lambertian.DS", TextureLayout::ShaderResource);
			node.AddInput("lambertian.GB_N", TextureLayout::ShaderResource);
			node.AddInput("lambertian.GB_MAT", TextureLayout::ShaderResource);
			node.AddInput("lambertian.GB_MV", TextureLayout::ShaderResource);
			//node.AddInput("ssao.SB", TextureLayout::ShaderResource); // Env map
			//node.AddInput("lambertian.GB_C", TextureLayout::ShaderResource); // BRDF LUT
			node.AddOutput("RT", TextureLayout::ShaderResource, "rawScene");
			node.AddOutput("SSR", TextureLayout::UnorderedAccess, "ssr");
			node.SetHintCompute();
			graphDesc.RenderPasses.emplace_back(std::move(node));
		}
#pragma endregion
#pragma region Upscaling
		{
			RenderNode node("upscale", "fsr1", RenderPass::UpscaleFSR1::GetDesc(graphDesc.GetFormat("upscaledScene")), PassExecutionType::Processor);
			node.AddInput("wireframe.RT", TextureLayout::ShaderResource);
			node.AddOutput("RT", TextureLayout::UnorderedAccess, "upscaledScene", "rawScene");
			node.SetHintCompute();
			graphDesc.RenderPasses.emplace_back(std::move(node));
		}
		{
			RenderNode node("upscale", "fsr2", RenderPass::UpscaleFSR2::GetDesc(), PassExecutionType::Processor);
			node.AddInput("wireframe.RT", TextureLayout::ShaderResource);
			node.AddInput("wireframe.DS", TextureLayout::ShaderResource);
			node.AddInput("lambertian.GB_MV", TextureLayout::ShaderResource);
			node.AddInput("lambertian.GB_R", TextureLayout::ShaderResource);
			node.AddOutput("RT", TextureLayout::UnorderedAccess, "upscaledScene", "rawScene");
			node.SetHintCompute();
			graphDesc.RenderPasses.emplace_back(std::move(node));
		}
		{
			RenderNode node("upscale", "xess", RenderPass::UpscaleXeSS::GetDesc(), PassExecutionType::Processor);
			node.AddInput("wireframe.RT", TextureLayout::ShaderResource);
			node.AddInput("wireframe.DS", TextureLayout::ShaderResource);
			node.AddInput("lambertian.GB_MV", TextureLayout::ShaderResource);
			node.AddInput("lambertian.GB_R", TextureLayout::ShaderResource);
			node.AddOutput("RT", TextureLayout::UnorderedAccess, "upscaledScene", "rawScene");
			node.SetHintCompute();
			graphDesc.RenderPasses.emplace_back(std::move(node));
		}
		{
			RenderNode node("upscale", "nis", RenderPass::UpscaleNIS::GetDesc(), PassExecutionType::Processor);
			node.AddInput("wireframe.RT", TextureLayout::ShaderResource);
			node.AddOutput("RT", TextureLayout::UnorderedAccess, "upscaledScene", "rawScene");
			node.SetHintCompute();
			graphDesc.RenderPasses.emplace_back(std::move(node));
		}
#pragma endregion
#pragma region Post process display size
		{
			RenderPass::ClearBuffer<2>::ExecuteData clearInfo;
			clearInfo.Info[0].BufferType = RenderPass::ClearBufferType::RTV;
			clearInfo.Info[0].ClearValue.Color = ColorF4(0.0f, 0.0f, 0.0f, 0.0f);
			clearInfo.Info[1].BufferType = RenderPass::ClearBufferType::DSV;
			clearInfo.Info[1].ClearValue.DSV.Depth = 0.0f;
			clearInfo.Info[1].ClearValue.DSV.Stencil = 0;

			RenderNode node("outlineClear", "", RenderPass::ClearBuffer<2>::GetDesc(static_cast<PassType>(CorePassType::OutlineClear),
				clearInfo), PassExecutionType::Producer);
			node.AddOutput("RT", TextureLayout::RenderTarget, "outline");
			node.AddOutput("DS", TextureLayout::DepthStencilWrite, "outlineDepth");
			graphDesc.RenderPasses.emplace_back(std::move(node));
		}
		{
			RenderNode node("outlineDraw", "", RenderPass::OutlineDraw::GetDesc(graphDesc.GetFormat("outline"),
				graphDesc.GetFormat("outlineDepth")), PassExecutionType::DynamicProcessor);
			node.AddInput("outlineClear.RT", TextureLayout::RenderTarget);
			node.AddInput("outlineClear.DS", TextureLayout::DepthStencilWrite);
			node.AddOutput("RT", TextureLayout::RenderTarget, "outline");
			node.AddOutput("DS", TextureLayout::DepthStencilWrite, "outlineDepth");
			node.SetHintGfx();
			graphDesc.RenderPasses.emplace_back(std::move(node));
		}
		{
			RenderNode node("horizontalBlur", "", RenderPass::HorizontalBlur::GetDesc(graphDesc.GetFormat("outlineBlur")), PassExecutionType::Processor);
			node.AddInput("outlineDraw.RT", TextureLayout::ShaderResource);
			node.AddOutput("RT", TextureLayout::RenderTarget, "outlineBlur");
			node.SetHintGfx();
			graphDesc.RenderPasses.emplace_back(std::move(node));
		}
		{
			RenderNode node("verticalBlur", "", RenderPass::VerticalBlur::GetDesc(graphDesc.GetFormat("upscaledScene"),
				graphDesc.GetFormat("outlineDepth")), PassExecutionType::Processor);
			node.AddInput("horizontalBlur.RT", TextureLayout::ShaderResource);
			node.AddInput("upscale.RT", TextureLayout::RenderTarget);
			node.AddInput("outlineDraw.DS", TextureLayout::DepthStencilRead);
			node.AddOutput("RT", TextureLayout::RenderTarget, "upscaledScene");
			node.SetHintGfx();
			graphDesc.RenderPasses.emplace_back(std::move(node));
		}
		{
			RenderNode node("hdrGamma", "", RenderPass::HDRGammaCorrection::GetDesc(Settings::BackbufferFormat), PassExecutionType::Processor);
			node.AddInput("verticalBlur.RT", TextureLayout::ShaderResource);
			node.AddOutput("RT", TextureLayout::RenderTarget, BACKBUFFER_NAME);
			node.SetHintGfx();
			graphDesc.RenderPasses.emplace_back(std::move(node));
		}
#pragma endregion
		return graphDesc;
	}
}