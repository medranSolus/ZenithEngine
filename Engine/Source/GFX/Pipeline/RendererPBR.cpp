#include "GFX/Pipeline/RendererPBR.h"
#include "GFX/Pipeline/RenderPasses.h"

#define ZE_MAKE_NODE(name, queueType, passNamespace, ...) RenderNode node(name, queueType, RenderPass::passNamespace::Execute, RenderPass::passNamespace::Clean, RenderPass::passNamespace::Setup(__VA_ARGS__))

namespace ZE::GFX::Pipeline
{
	void RendererPBR::SetupRenderSlots(RendererBuildData& buildData) noexcept
	{
		buildData.RendererSlots.AddRange(
			{
				1, 13, 0,
				Resource::ShaderType::Pixel | Resource::ShaderType::Compute,
				Binding::RangeFlag::CBV | Binding::RangeFlag::GlobalBuffer
			});

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

		buildData.RendererSlots.Samplers.reserve(sizeof(TYPES) * sizeof(ADDRESS_MODES));

		U32 slot = 0;
		for (U8 type = 0; type < sizeof(TYPES); ++type)
		{
			for (U8 address = 0; address < sizeof(ADDRESS_MODES); ++address)
			{
				buildData.RendererSlots.AddSampler(
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
	}

	constexpr void RendererPBR::SetupBlurKernel() noexcept
	{
		float sum = 0.0f;
		for (S32 i = 0; i <= settingsData.BlurRadius; ++i)
		{
			const float g = Math::Gauss(Utils::SafeCast<float>(i), blurSigma);
			sum += g;
			settingsData.BlurCoefficients[i].x = g;
		}
		for (S32 i = 0; i <= settingsData.BlurRadius; ++i)
			settingsData.BlurCoefficients[i].x /= sum;
	}

	constexpr void RendererPBR::SetupBlurIntensity() noexcept
	{
		if (settingsData.HDRExposure < 1.0f)
			settingsData.BlurIntensity = 1.0f / settingsData.HDRExposure;
		else
			settingsData.BlurIntensity = 1.0f;
	}

	constexpr void RendererPBR::SetupBlurData(U32 width, U32 height) noexcept
	{
		settingsData.BlurRadius = DataPBR::BLUR_KERNEL_RADIUS;
		settingsData.BlurWidth = width;
		settingsData.BlurHeight = height;
		SetupBlurIntensity();
		SetupBlurKernel();
	}

	constexpr void RendererPBR::SetupXeGTAOQuality() noexcept
	{
		switch (ssaoSettings.xegtao.Settings.QualityLevel)
		{
		default:
			ZE_FAIL("Unknown XeGTAO quality level!");
			[[fallthrough]];
		case 0: // Low
		{
			ssaoSettings.xegtao.SliceCount = 1.0f;
			ssaoSettings.xegtao.StepsPerSlice = 2.0f;
			break;
		}
		case 1: // Medium
		{
			ssaoSettings.xegtao.SliceCount = 2.0f;
			ssaoSettings.xegtao.StepsPerSlice = 2.0f;
			break;
		}
		case 2: // High
		{
			ssaoSettings.xegtao.SliceCount = 3.0f;
			ssaoSettings.xegtao.StepsPerSlice = 3.0f;
			break;
		}
		case 3: // Ultra
		{
			ssaoSettings.xegtao.SliceCount = 9.0f;
			ssaoSettings.xegtao.StepsPerSlice = 3.0f;
			break;
		}
		}
	}

	constexpr void RendererPBR::SetupSSAOData() noexcept
	{
		switch (Settings::GetAOType())
		{
		default:
			ZE_ENUM_UNHANDLED();
		case AOType::None:
			break;
		case AOType::XeGTAO:
		{
			ssaoSettings.xegtao = {};
			ssaoSettings.xegtao.Settings.DenoisePasses = 1;
			SetupXeGTAOQuality();
			break;
		}
		case AOType::CACAO:
		{
			ssaoSettings.cacao = FFX_CACAO_DEFAULT_SETTINGS;
			ssaoSettings.cacao.generateNormals = false;
			break;
		}
		}
	}

	void RendererPBR::Init(Device& dev, CommandList& mainList, Data::AssetsStreamer& assets, const ParamsPBR& params)
	{
		const UInt2 outlineBuffSizes = { Settings::DisplaySize.X / 2, Settings::DisplaySize.Y / 2 };
		FrameBufferDesc frameBufferDesc;
		frameBufferDesc.Init(15);

#pragma region Framebuffer definition
		// GBuffer related resources
		RID gbuffNormalCompute = 0;
		RID gbuffDepthCompute = 0;
		if (Settings::GetAOType() != AOType::None)
		{
			gbuffNormalCompute = frameBufferDesc.AddResource(
				{ Settings::RenderSize, 1, FrameResourceFlags::None, PixelFormat::R16G16_Float, ColorF4() });
			gbuffDepthCompute = frameBufferDesc.AddResource(
				{ Settings::RenderSize, 1, FrameResourceFlags::ForceDSV, PixelFormat::DepthOnly, ColorF4(), 0.0f, 0 });
		}
		RID gbuffMotion = INVALID_RID;
		if (Settings::ComputeMotionVectors())
			gbuffMotion = frameBufferDesc.AddResource({ Settings::RenderSize, 1, FrameResourceFlags::None, PixelFormat::R16G16_Float, ColorF4() });
		RID gbuffReactive = INVALID_RID;
		if (Settings::GetUpscaler() == UpscalerType::Fsr2 || Settings::GetUpscaler() == UpscalerType::XeSS)
			gbuffReactive = frameBufferDesc.AddResource({ Settings::RenderSize, 1, FrameResourceFlags::None, PixelFormat::R8_UNorm, ColorF4() });

		const RID gbuffColor = frameBufferDesc.AddResource(
			{ Settings::RenderSize, 1, FrameResourceFlags::None, PixelFormat::R8G8B8A8_UNorm, ColorF4() });
		const RID gbuffNormal = frameBufferDesc.AddResource(
			{ Settings::RenderSize, 1, FrameResourceFlags::None, PixelFormat::R16G16_Float, ColorF4() });
		const RID gbuffSpecular = frameBufferDesc.AddResource(
			{ Settings::RenderSize, 1, FrameResourceFlags::None, PixelFormat::R16G16B16A16_Float, ColorF4() });
		const RID gbuffDepth = frameBufferDesc.AddResource(
			{ Settings::RenderSize, 1, FrameResourceFlags::ForceSRV, PixelFormat::DepthOnly, ColorF4(), 0.0f, 0 });

		// Light buffer related resources
		const RID lightbuffColor = frameBufferDesc.AddResource(
			{ Settings::RenderSize, 1, FrameResourceFlags::None, PixelFormat::R16G16B16A16_Float, ColorF4(0.0f, 0.0f, 0.0f, 0.0f) });
		const RID lightbuffSpecular = frameBufferDesc.AddResource(
			{ Settings::RenderSize, 1, FrameResourceFlags::None, PixelFormat::R16G16B16A16_Float, ColorF4(0.0f, 0.0f, 0.0f, 0.0f) });
		RID ssao = INVALID_RID;
		if (Settings::GetAOType() != AOType::None)
			ssao = frameBufferDesc.AddResource({ Settings::RenderSize, 1, FrameResourceFlags::ForceSRV, PixelFormat::R8_UInt, ColorF4() });

		// Combined scene related resources
		const RID rawScene = frameBufferDesc.AddResource(
			{ Settings::RenderSize, 1, FrameResourceFlags::None, PixelFormat::R16G16B16A16_Float, ColorF4() });
		RID ssr = INVALID_RID;
		if (Settings::IsEnabledSSSR())
			ssr = frameBufferDesc.AddResource({ Settings::RenderSize, 1, FrameResourceFlags::None, PixelFormat::R16G16B16A16_Float, ColorF4() });
		RID upscaledScene = rawScene;
		if (Settings::GetUpscaler() != UpscalerType::None)
			upscaledScene = frameBufferDesc.AddResource({ Settings::DisplaySize, 1, FrameResourceFlags::None, PixelFormat::R16G16B16A16_Float, ColorF4() });

		// Outline related resources
		const RID outline = frameBufferDesc.AddResource(
			{ outlineBuffSizes, 1, FrameResourceFlags::None, Settings::BackbufferFormat, ColorF4(0.0f, 0.0f, 0.0f, 0.0f) });
		const RID outlineBlur = frameBufferDesc.AddResource(
			{ outlineBuffSizes, 1, FrameResourceFlags::None, Settings::BackbufferFormat, ColorF4(0.0f, 0.0f, 0.0f, 0.0f) });
		const RID outlineDepth = frameBufferDesc.AddResource(
			{ Settings::DisplaySize, 1, FrameResourceFlags::None, PixelFormat::DepthStencil, ColorF4(), 0.0f, 0 });
#pragma endregion

		std::vector<GFX::Pipeline::RenderNode> nodes;
		RendererBuildData buildData = { execData.Bindings, assets };
		SetupRenderSlots(buildData);

		blurSigma = params.Sigma;
		settingsData.DisplaySize = Settings::DisplaySize;
		settingsData.RenderSize = Settings::RenderSize;
		settingsData.Gamma = params.Gamma;
		settingsData.GammaInverse = 1.0f / params.Gamma;
		settingsData.AmbientLight = { 0.05f, 0.05f, 0.05f };
		settingsData.HDRExposure = params.HDRExposure;
		settingsData.ShadowMapSize = Utils::SafeCast<float>(params.ShadowMapSize);
		settingsData.ShadowBias = Utils::SafeCast<float>(params.ShadowBias) / settingsData.ShadowMapSize;
		settingsData.ShadowNormalOffset = params.ShadowNormalOffset;
		settingsData.MipBias = CalculateMipBias(Settings::RenderSize.X, Settings::DisplaySize.X, Settings::GetUpscaler());
		switch (Settings::GetUpscaler())
		{
		case UpscalerType::Fsr2:
			settingsData.ReactiveMaskClamp = 0.9f;
			break;
		case UpscalerType::XeSS:
			settingsData.ReactiveMaskClamp = 1.0f;
			break;
		default:
			settingsData.ReactiveMaskClamp = 0.0f;
			break;
		}
		SetupBlurData(outlineBuffSizes.X, outlineBuffSizes.Y);
		SetupSSAOData();
		execData.SettingsBuffer.Init(dev, assets.GetDisk(), { INVALID_EID, &settingsData, nullptr, sizeof(DataPBR) });

#pragma region Geometry
		{
			ZE_MAKE_NODE("lambertian", QueueType::Main, Lambertian, dev, buildData, frameBufferDesc.GetFormat(gbuffDepth),
				frameBufferDesc.GetFormat(gbuffColor), frameBufferDesc.GetFormat(gbuffNormal), frameBufferDesc.GetFormat(gbuffSpecular),
				frameBufferDesc.GetFormat(gbuffMotion), frameBufferDesc.GetFormat(gbuffReactive));
			node.AddOutput("DS", Resource::StateDepthWrite, gbuffDepth);
			node.AddOutput("GB_C", Resource::StateRenderTarget, gbuffColor);
			node.AddOutput("GB_N", Resource::StateRenderTarget, gbuffNormal);
			node.AddOutput("GB_S", Resource::StateRenderTarget, gbuffSpecular);
			node.AddOutput("GB_MV", Resource::StateRenderTarget, gbuffMotion);
			node.AddOutput("GB_R", Resource::StateRenderTarget, gbuffReactive);
			nodes.emplace_back(std::move(node));
		}
		if (Settings::GetAOType() != AOType::None)
		{
			RenderNode node("lambertianComputeCopy", QueueType::Main, RenderPass::LambertianComputeCopy::Execute);
			node.AddInput("lambertian.GB_N", Resource::StateCopySource);
			node.AddInput("lambertian.DS", Resource::StateCopySource);
			node.AddOutput("GB_N", Resource::StateCopyDestination, gbuffNormalCompute);
			node.AddOutput("DS", Resource::StateCopyDestination, gbuffDepthCompute);
			nodes.emplace_back(std::move(node));
		}
#pragma endregion
#pragma region Lightning
		{
			ZE_MAKE_NODE("dirLight", QueueType::Main, DirectionalLight, dev, buildData,
				frameBufferDesc.GetFormat(lightbuffColor), frameBufferDesc.GetFormat(lightbuffSpecular),
				PixelFormat::R32_Float, PixelFormat::DepthOnly);
			node.AddInput("lambertian.GB_N", Resource::StateShaderResourcePS);
			node.AddInput("lambertian.GB_S", Resource::StateShaderResourcePS);
			node.AddInput("lambertian.DS", Resource::StateShaderResourcePS);
			node.AddInnerBuffer(Resource::StateRenderTarget,
				{ params.ShadowMapSize, params.ShadowMapSize, 1, FrameResourceFlags::ForceSRV, PixelFormat::R32_Float, { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX } });
			node.AddInnerBuffer(Resource::StateDepthWrite,
				{ params.ShadowMapSize, params.ShadowMapSize, 1, FrameResourceFlags::None, PixelFormat::DepthOnly, ColorF4(), 0.0f, 0 });
			node.AddOutput("LB_C", Resource::StateRenderTarget, lightbuffColor);
			node.AddOutput("LB_S", Resource::StateRenderTarget, lightbuffSpecular);
			nodes.emplace_back(std::move(node));
		}
		{
			ZE_MAKE_NODE("spotLight", QueueType::Main, SpotLight, dev, buildData,
				frameBufferDesc.GetFormat(lightbuffColor), frameBufferDesc.GetFormat(lightbuffSpecular),
				PixelFormat::R32_Float, PixelFormat::DepthOnly);
			node.AddInput("lambertian.GB_N", Resource::StateShaderResourcePS);
			node.AddInput("lambertian.GB_S", Resource::StateShaderResourcePS);
			node.AddInput("lambertian.DS", Resource::StateShaderResourcePS);
			node.AddInput("dirLight.LB_C", Resource::StateRenderTarget);
			node.AddInput("dirLight.LB_S", Resource::StateRenderTarget);
			node.AddInnerBuffer(Resource::StateRenderTarget,
				{ params.ShadowMapSize, params.ShadowMapSize, 1, FrameResourceFlags::ForceSRV, PixelFormat::R32_Float, { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX } });
			node.AddInnerBuffer(Resource::StateDepthWrite,
				{ params.ShadowMapSize, params.ShadowMapSize, 1, FrameResourceFlags::None, PixelFormat::DepthOnly, ColorF4(), 0.0f, 0 });
			node.AddOutput("LB_C", Resource::StateRenderTarget, lightbuffColor);
			node.AddOutput("LB_S", Resource::StateRenderTarget, lightbuffSpecular);
			nodes.emplace_back(std::move(node));
		}
		{
			ZE_MAKE_NODE("pointLight", QueueType::Main, PointLight, dev, buildData,
				frameBufferDesc.GetFormat(lightbuffColor), frameBufferDesc.GetFormat(lightbuffSpecular),
				PixelFormat::R32_Float, PixelFormat::DepthOnly);
			node.AddInput("lambertian.GB_N", Resource::StateShaderResourcePS);
			node.AddInput("lambertian.GB_S", Resource::StateShaderResourcePS);
			node.AddInput("lambertian.DS", Resource::StateShaderResourcePS);
			node.AddInput("spotLight.LB_C", Resource::StateRenderTarget);
			node.AddInput("spotLight.LB_S", Resource::StateRenderTarget);
			node.AddInnerBuffer(Resource::StateRenderTarget,
				{ params.ShadowMapSize, params.ShadowMapSize, 1, FrameResourceFlags::Cube | FrameResourceFlags::ForceSRV, PixelFormat::R32_Float, { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX } });
			node.AddInnerBuffer(Resource::StateDepthWrite,
				{ params.ShadowMapSize, params.ShadowMapSize, 1, FrameResourceFlags::Cube, PixelFormat::DepthOnly, ColorF4(), 0.0f, 0 });
			node.AddOutput("LB_C", Resource::StateRenderTarget, lightbuffColor);
			node.AddOutput("LB_S", Resource::StateRenderTarget, lightbuffSpecular);
			nodes.emplace_back(std::move(node));
		}
		switch (Settings::GetAOType())
		{
		default:
			ZE_ENUM_UNHANDLED();
		case AOType::None:
			break;
		case AOType::XeGTAO:
		{
			ZE_MAKE_NODE("ssao", QueueType::Compute, XeGTAO, dev, buildData);
			node.AddInput("lambertianComputeCopy.DS", Resource::StateShaderResourceNonPS);
			node.AddInput("lambertianComputeCopy.GB_N", Resource::StateShaderResourceNonPS);
			node.AddInnerBuffer(Resource::StateUnorderedAccess,
				{ Settings::RenderSize, 1, FrameResourceFlags::ForceSRV, PixelFormat::R32_Float, ColorF4(), 0.0f, 0, 5 });
			node.AddInnerBuffer(Resource::StateUnorderedAccess,
				{ Settings::RenderSize, 1, FrameResourceFlags::ForceSRV, frameBufferDesc.GetFormat(ssao), ColorF4() });
			node.AddInnerBuffer(Resource::StateUnorderedAccess,
				{ Settings::RenderSize, 1, FrameResourceFlags::ForceSRV, PixelFormat::R8_UNorm, ColorF4() });
			node.AddOutput("SB", Resource::StateUnorderedAccess, ssao);
			nodes.emplace_back(std::move(node));
			break;
		}
		case AOType::CACAO:
		{
			ZE_MAKE_NODE("ssao", QueueType::Compute, CACAO, dev, buildData, Settings::RenderSize.X, Settings::RenderSize.Y);
			node.AddInput("lambertianComputeCopy.DS", Resource::StateShaderResourceNonPS);
			node.AddInput("lambertianComputeCopy.GB_N", Resource::StateShaderResourceNonPS);
			node.AddOutput("SB", Resource::StateUnorderedAccess, ssao);
			nodes.emplace_back(std::move(node));
			break;
		}
		}
		{
			ZE_MAKE_NODE("lightCombine", QueueType::Main, LightCombine, dev, buildData, frameBufferDesc.GetFormat(rawScene));
			node.AddInput("lambertian.GB_C", Resource::StateShaderResourcePS);
			node.AddInput("pointLight.LB_C", Resource::StateShaderResourcePS);
			node.AddInput("pointLight.LB_S", Resource::StateShaderResourcePS);
			node.AddInput("ssao.SB", Resource::StateShaderResourcePS, false);
			node.AddOutput("RT", Resource::StateRenderTarget, rawScene);
			nodes.emplace_back(std::move(node));
		}
#pragma endregion
#pragma region Render size post process
		{
			ZE_MAKE_NODE("skybox", QueueType::Main, Skybox, dev, buildData,
				frameBufferDesc.GetFormat(rawScene), frameBufferDesc.GetFormat(gbuffDepth),
				params.SkyboxPath, params.SkyboxExt);
			node.AddInput("lightCombine.RT", Resource::StateRenderTarget);
			node.AddInput("lambertian.DS", Resource::StateDepthRead);
			node.AddOutput("RT", Resource::StateRenderTarget, rawScene);
			node.AddOutput("DS", Resource::StateDepthRead, gbuffDepth);
			nodes.emplace_back(std::move(node));
		}
		{
			ZE_MAKE_NODE("wireframe", QueueType::Main, Wireframe, dev, buildData,
				frameBufferDesc.GetFormat(rawScene), frameBufferDesc.GetFormat(gbuffDepth));
			node.AddInput("skybox.RT", Resource::StateRenderTarget);
			node.AddInput("skybox.DS", Resource::StateDepthWrite);
			node.AddOutput("RT", Resource::StateRenderTarget, rawScene);
			node.AddOutput("DS", Resource::StateDepthWrite, gbuffDepth);
			nodes.emplace_back(std::move(node));
		}
		if (Settings::IsEnabledSSSR())
		{
			ZE_MAKE_NODE("sssr", QueueType::Main, SSSR, dev, buildData, Settings::RenderSize.X, Settings::RenderSize.Y);
			node.AddInput("wireframe.RT", Resource::StateShaderResourceNonPS);
			node.AddInput("lambertian.DS", Resource::StateShaderResourceNonPS);
			node.AddInput("lambertian.GB_MV", Resource::StateShaderResourceNonPS);
			//node.AddInput("lambertian.GB_N", Resource::StateShaderResourceNonPS);
			//node.AddInput("pointLight.LB_S", Resource::StateShaderResourceNonPS); // Roughness
			//node.AddInput("ssao.SB", Resource::StateShaderResourceNonPS); // Env map
			//node.AddInput("lambertian.GB_C", Resource::StateShaderResourceNonPS); // BRDF LUT
			node.AddOutput("RT", Resource::StateShaderResourceNonPS, rawScene);
			node.AddOutput("SSR", Resource::StateUnorderedAccess, ssr);
			nodes.emplace_back(std::move(node));
		}
#pragma endregion
#pragma region Upscaling
		switch (Settings::GetUpscaler())
		{
		default:
			ZE_ENUM_UNHANDLED();
		case UpscalerType::None:
			break;
		case UpscalerType::Fsr1:
		{
			ZE_MAKE_NODE("upscale", QueueType::Main, UpscaleFSR1, dev, frameBufferDesc.GetFormat(upscaledScene));
			node.AddInput((Settings::IsEnabledSSSR() ? "sssr.RT" : "wireframe.RT"), Resource::StateShaderResourceNonPS);
			node.AddOutput("RT", Resource::StateUnorderedAccess, upscaledScene);
			nodes.emplace_back(std::move(node));
			break;
		}
		case UpscalerType::Fsr2:
		{
			ZE_MAKE_NODE("upscale", QueueType::Main, UpscaleFSR2, dev);
			node.AddInput((Settings::IsEnabledSSSR() ? "sssr.RT" : "wireframe.RT"), Resource::StateShaderResourceNonPS);
			node.AddInput("wireframe.DS", Resource::StateShaderResourceNonPS);
			node.AddInput("lambertian.GB_MV", Resource::StateShaderResourceNonPS);
			node.AddInput("lambertian.GB_R", Resource::StateShaderResourceNonPS);
			node.AddOutput("RT", Resource::StateUnorderedAccess, upscaledScene);
			nodes.emplace_back(std::move(node));
			break;
		}
		case UpscalerType::XeSS:
		{
			RenderNode node("upscale", QueueType::Main, RenderPass::UpscaleXeSS::Execute, nullptr, RenderPass::UpscaleXeSS::Setup(dev));
			node.AddInput((Settings::IsEnabledSSSR() ? "sssr.RT" : "wireframe.RT"), Resource::StateShaderResourceNonPS);
			node.AddInput("wireframe.DS", Resource::StateShaderResourceNonPS);
			node.AddInput("lambertian.GB_MV", Resource::StateShaderResourceNonPS);
			node.AddInput("lambertian.GB_R", Resource::StateShaderResourceNonPS);
			node.AddOutput("RT", Resource::StateUnorderedAccess, upscaledScene);
			nodes.emplace_back(std::move(node));
			break;
		}
		case UpscalerType::NIS:
		{
			ZE_MAKE_NODE("upscale", QueueType::Main, UpscaleNIS, dev, buildData);
			node.AddInput((Settings::IsEnabledSSSR() ? "sssr.RT" : "wireframe.RT"), Resource::StateShaderResourceNonPS);
			node.AddOutput("RT", Resource::StateUnorderedAccess, upscaledScene);
			nodes.emplace_back(std::move(node));
			break;
		}
		}
#pragma endregion
#pragma region Display size post process
		{
			ZE_MAKE_NODE("outlineDraw", QueueType::Main, OutlineDraw, dev, buildData,
				frameBufferDesc.GetFormat(outline), frameBufferDesc.GetFormat(outlineDepth));
			node.AddOutput("RT", Resource::StateRenderTarget, outline);
			node.AddOutput("DS", Resource::StateDepthWrite, outlineDepth);
			nodes.emplace_back(std::move(node));
		}
		{
			ZE_MAKE_NODE("horizontalBlur", QueueType::Main, HorizontalBlur, dev, buildData, frameBufferDesc.GetFormat(outlineBlur));
			node.AddInput("outlineDraw.RT", Resource::StateShaderResourcePS);
			node.AddOutput("RT", Resource::StateRenderTarget, outlineBlur);
			nodes.emplace_back(std::move(node));
		}
		{
			ZE_MAKE_NODE("verticalBlur", QueueType::Main, VerticalBlur, dev, buildData, frameBufferDesc.GetFormat(upscaledScene), frameBufferDesc.GetFormat(outlineDepth));
			node.AddInput("horizontalBlur.RT", Resource::StateShaderResourcePS);
			node.AddInput(Settings::GetUpscaler() != UpscalerType::None ? "upscale.RT" : (Settings::IsEnabledSSSR() ? "sssr.RT" : "wireframe.RT"), Resource::StateRenderTarget);
			node.AddInput("outlineDraw.DS", Resource::StateDepthRead);
			node.AddOutput("RT", Resource::StateRenderTarget, upscaledScene);
			nodes.emplace_back(std::move(node));
		}
		{
			ZE_MAKE_NODE("hdrGamma", QueueType::Main, HDRGammaCorrection, dev, buildData, Settings::BackbufferFormat);
			node.AddInput("verticalBlur.RT", Resource::StateShaderResourcePS);
			node.AddOutput("RT", Resource::StateRenderTarget, BACKBUFFER_RID);
			nodes.emplace_back(std::move(node));
		}
#pragma endregion
		Finalize(dev, mainList, nodes, frameBufferDesc, buildData, params.MinimizeRenderPassDistances);
	}

	void RendererPBR::UpdateSettingsData(const Data::Projection& projection) noexcept
	{
		currentProjectionData = projection;
		// No need to create new projection as it's data is always changing with jitter
		if (!Settings::ApplyJitter())
			Math::XMStoreFloat4x4(&currentProjection, Data::GetProjectionMatrix(projection));
		dynamicData.JitterCurrent = { 0.0f, 0.0f };
		// Not uploading now since it's uploaded every frame
		dynamicData.NearClip = currentProjectionData.NearClip;
	}

	void RendererPBR::SetInverseViewProjection(EID camera) noexcept
	{
		Data::Camera& camData = Settings::Data.get<Data::Camera>(camera);
		UpdateSettingsData(camData.Projection);

		const auto& transform = Settings::Data.get<Data::Transform>(camera); // TODO: Change into TransformGlobal later
		Math::XMStoreFloat4x4(&dynamicData.ViewProjectionInverseTps, Math::XMMatrixTranspose(Math::XMMatrixInverse(nullptr,
			Math::XMMatrixLookToLH(Math::XMLoadFloat3(&transform.Position),
				Math::XMLoadFloat3(&camData.EyeDirection),
				Math::XMLoadFloat3(&camData.UpVector)) * Data::GetProjectionMatrix(camData.Projection))));

		if (Settings::ApplyJitter())
			CalculateJitter(jitterIndex, dynamicData.JitterCurrent.x, dynamicData.JitterCurrent.y, Settings::RenderSize, Settings::GetUpscaler());
	}

	void RendererPBR::UpdateWorldData(Device& dev, EID camera) noexcept
	{
		ZE_ASSERT((Settings::Data.all_of<Data::TransformGlobal, Data::Camera>(camera)),
			"Current camera does not have all required components!");

		const auto& currentCamera = Settings::Data.get<Data::Camera>(camera);
		const auto& transform = Settings::Data.get<Data::Transform>(camera); // TODO: Change into TransformGlobal later
		cameraRotation = transform.Rotation;

		// Setup shader world data
		dynamicData.CameraPos = transform.Position;
		const Matrix view = Math::XMMatrixLookToLH(Math::XMLoadFloat3(&dynamicData.CameraPos),
			Math::XMLoadFloat3(&currentCamera.EyeDirection),
			Math::XMLoadFloat3(&currentCamera.UpVector));
		Math::XMStoreFloat4x4(&dynamicData.ViewTps, Math::XMMatrixTranspose(view));

		if (Settings::ComputeMotionVectors())
			prevViewProjectionTps = dynamicData.ViewProjectionTps;

		Matrix projection;
		if (Settings::ApplyJitter())
		{
			CalculateJitter(jitterIndex, currentProjectionData.JitterX, currentProjectionData.JitterY, Settings::RenderSize, Settings::GetUpscaler());
			dynamicData.JitterPrev = dynamicData.JitterCurrent;
			dynamicData.JitterCurrent = { currentProjectionData.JitterX, currentProjectionData.JitterY };

			projection = Data::GetProjectionMatrix(currentProjectionData);
			Math::XMStoreFloat4x4(&currentProjection, projection);
		}
		else
			projection = Math::XMLoadFloat4x4(&currentProjection);

		const Matrix viewProjection = view * projection;
		Math::XMStoreFloat4x4(&dynamicData.ViewProjectionTps, Math::XMMatrixTranspose(viewProjection));
		Math::XMStoreFloat4x4(&dynamicData.ViewProjectionInverseTps, Math::XMMatrixTranspose(Math::XMMatrixInverse(nullptr, viewProjection)));
	}

	void RendererPBR::ShowWindow(Device& dev, Data::AssetsStreamer& assets)
	{
		bool change = false;
		if (ImGui::CollapsingHeader("Outline"))
		{
			ImGui::Columns(2, "##outline_options", false);
			{
				ImGui::Text("Blur radius");
				ImGui::SetNextItemWidth(-1.0f);
				change |= ImGui::SliderInt("##blur_radius", &settingsData.BlurRadius, 1, DataPBR::BLUR_KERNEL_RADIUS);
			}
			ImGui::NextColumn();
			{
				ImGui::Text("Outline range");
				ImGui::SetNextItemWidth(-1.0f);
				if (GUI::InputClamp(0.1f, 25.0f, blurSigma,
					ImGui::InputFloat("##blur_sigma", &blurSigma, 0.1f, 0.0f, "%.1f")))
				{
					change = true;
					SetupBlurKernel();
				}
			}
			ImGui::Columns(1);
			ImGui::NewLine();
		}
		if (ImGui::CollapsingHeader("Display"))
		{
			ImGui::Columns(2, "##display_options", false);
			{
				ImGui::Text("Gamma correction");
				ImGui::SetNextItemWidth(-1.0f);
				if (GUI::InputClamp(1.0f, 10.0f, settingsData.Gamma,
					ImGui::InputFloat("##gamma", &settingsData.Gamma, 0.1f, 0.0f, "%.1f")))
				{
					change = true;
					settingsData.GammaInverse = 1.0f / settingsData.Gamma;
				}
			}
			ImGui::NextColumn();
			{
				ImGui::Text("HDR exposure");
				ImGui::SetNextItemWidth(-1.0f);
				if (GUI::InputClamp(0.1f, FLT_MAX, settingsData.HDRExposure,
					ImGui::InputFloat("##hdr", &settingsData.HDRExposure, 0.1f, 0.0f, "%.1f")))
				{
					change = true;
					SetupBlurIntensity();
				}
			}
			ImGui::Columns(1);
			ImGui::NewLine();
		}
		if (ImGui::CollapsingHeader("Shadows"))
		{
			ImGui::Columns(2, "##shadow_options", false);
			{
				ImGui::Text("Depth bias");
				ImGui::SetNextItemWidth(-1.0f);
				S32 bias = Utils::SafeCast<S32>(settingsData.ShadowBias * settingsData.ShadowMapSize);
				if (ImGui::InputInt("##depth_bias", &bias))
				{
					change = true;
					settingsData.ShadowBias = Utils::SafeCast<float>(bias) / settingsData.ShadowMapSize;
				}
			}
			ImGui::NextColumn();
			{
				ImGui::Text("Normal offset");
				ImGui::SetNextItemWidth(-1.0f);
				change |= GUI::InputClamp(0.0f, 1.0f, settingsData.ShadowNormalOffset,
					ImGui::InputFloat("##normal_offset", &settingsData.ShadowNormalOffset, 0.001f, 0.0f, "%.3f"));
			}
			ImGui::Columns(1);

			ImGui::Text("Ambient color");
			ImGui::SetNextItemWidth(-5.0f);
			change |= ImGui::ColorEdit3("##ambient_color", reinterpret_cast<float*>(&settingsData.AmbientLight),
				ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoLabel);
			ImGui::NewLine();
		}
		switch (Settings::GetAOType())
		{
		default:
			ZE_ENUM_UNHANDLED();
		case AOType::None:
			break;
		case AOType::XeGTAO:
		{
			if (ImGui::CollapsingHeader("XeGTAO"))
			{
				// GTAOImGuiSettings() don't indicate if quality or denoise passes has been updated...
				const int quality = ssaoSettings.xegtao.Settings.QualityLevel;
				const int denoise = ssaoSettings.xegtao.Settings.DenoisePasses;
				XeGTAO::GTAOImGuiSettings(ssaoSettings.xegtao.Settings);
				change |= quality != ssaoSettings.xegtao.Settings.QualityLevel || denoise != ssaoSettings.xegtao.Settings.DenoisePasses;
				if (change)
					SetupXeGTAOQuality();
				ImGui::NewLine();
			}
			break;
		}
		case AOType::CACAO:
		{
			if (ImGui::CollapsingHeader("CACAO"))
			{
				constexpr std::array<const char*, 5> LEVELS = { "Lowest", "Low", "Medium", "High", "Highest" };
				if (ImGui::BeginCombo("Quality level", LEVELS.at(ssaoSettings.cacao.qualityLevel)))
				{
					for (FfxCacaoQuality i = FFX_CACAO_QUALITY_LOWEST; const char* level : LEVELS)
					{
						const bool selected = i == ssaoSettings.cacao.qualityLevel;
						if (ImGui::Selectable(level, selected))
							ssaoSettings.cacao.qualityLevel = i;
						if (selected)
							ImGui::SetItemDefaultFocus();
						i = static_cast<FfxCacaoQuality>(i + 1);
					}
					ImGui::EndCombo();
				}

				ImGui::Columns(2, "##cacao_options", false);
				{
					ImGui::Text("Blur radius");
					ImGui::SetNextItemWidth(-1.0f);
					GUI::InputClamp(0.1f, FLT_MAX, ssaoSettings.cacao.radius,
						ImGui::InputFloat("##cacao_blur_radius", &ssaoSettings.cacao.radius, 0.1f, 1.0f, "%.1f"));
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Size of the occlusion sphere");
				}
				ImGui::NextColumn();
				{
					ImGui::Text("Blur pass count");
					ImGui::SetNextItemWidth(-1.0f);
					ImGui::SliderInt("##cacao_blur_count", reinterpret_cast<int*>(&ssaoSettings.cacao.blurPassCount), 0, 8);
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Number of edge-sensitive smart blur passes to apply");
				}
				ImGui::Columns(1);

				if (ssaoSettings.cacao.qualityLevel != FFX_CACAO_QUALITY_HIGHEST)
					ImGui::BeginDisabled(true);
				ImGui::Text("Adaptative quality limit");
				ImGui::InputFloat("##cacao_adapt_limit", &ssaoSettings.cacao.adaptiveQualityLimit, 0.01f, 0.1f, "%.2f");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Only for highest quality level");
				if (ssaoSettings.cacao.qualityLevel != FFX_CACAO_QUALITY_HIGHEST)
					ImGui::EndDisabled();

				if (ImGui::CollapsingHeader("Advanced"))
				{
					ImGui::Columns(2, "##cacao_advanced", false);
					{
						ImGui::Text("Sharpness");
						GUI::InputClamp(0.0f, 1.0f, ssaoSettings.cacao.sharpness,
							ImGui::InputFloat("##cacao_sharpness", &ssaoSettings.cacao.sharpness, 0.01f, 0.1f, "%.2f"));
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("How much to bleed over edges; 1: not at all, 0.5: half-half; 0.0: completely ignore edges");
						ImGui::Text("Fade out start");
						GUI::InputClamp(0.0f, ssaoSettings.cacao.fadeOutTo, ssaoSettings.cacao.fadeOutFrom,
							ImGui::InputFloat("##cacao_fade_from", &ssaoSettings.cacao.fadeOutFrom, 1.0f, 5.0f, "%.1f"));
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Distance to start fading out the effect");
					}
					ImGui::NextColumn();
					{
						ImGui::Text("Horizontal angle treshold");
						GUI::InputClamp(0.0f, 0.2f, ssaoSettings.cacao.horizonAngleThreshold,
							ImGui::InputFloat("##cacao_horizon_angle", &ssaoSettings.cacao.horizonAngleThreshold, 0.01f, 0.0f, "%.2f"));
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Limits self-shadowing (makes the sampling area less of a hemisphere, more of a spherical cone, to avoid self-shadowing and various artifacts due to low tessellation and depth buffer imprecision, etc.)");
						ImGui::Text("Fade out end");
						GUI::InputClamp(ssaoSettings.cacao.fadeOutFrom, FLT_MAX, ssaoSettings.cacao.fadeOutTo,
							ImGui::InputFloat("##cacao_fade_to", &ssaoSettings.cacao.fadeOutTo, 1.0f, 5.0f, "%.1f"));
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Distance at which the effect is faded out");
					}
					ImGui::Columns(1);

					ImGui::Text("Shadow controls");
					ImGui::Columns(2, "##cacao_shadows", false);
					{
						ImGui::Text("Shadow multipler");
						GUI::InputClamp(0.0f, 5.0f, ssaoSettings.cacao.shadowMultiplier,
							ImGui::InputFloat("##cacao_shadow_mult", &ssaoSettings.cacao.shadowMultiplier, 0.1f, 1.0f, "%.1f"));
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Effect strength linear multiplier");
						ImGui::Text("Shadow clamp");
						GUI::InputClamp(0.0f, 1.0f, ssaoSettings.cacao.shadowClamp,
							ImGui::InputFloat("##cacao_shadow_clamp", &ssaoSettings.cacao.shadowClamp, 0.01f, 0.1f, "%.2f"));
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Effect max limit (applied after multiplier but before blur)");
					}
					ImGui::NextColumn();
					{
						ImGui::Text("Shadow power");
						GUI::InputClamp(0.5f, 5.0f, ssaoSettings.cacao.shadowPower,
							ImGui::InputFloat("##cacao_shadow_pow", &ssaoSettings.cacao.shadowPower, 0.01f, 0.1f, "%.2f"));
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Effect strength modifier");
						ImGui::Text("Shadow detail");
						GUI::InputClamp(0.5f, 5.0f, ssaoSettings.cacao.detailShadowStrength,
							ImGui::InputFloat("##cacao_shadow_det", &ssaoSettings.cacao.detailShadowStrength, 0.1f, 1.0f, "%.1f"));
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Used for high-res detail AO using neighboring depth pixels: adds a lot of detail but also reduces temporal stability (adds aliasing)");
					}
					ImGui::Columns(1);

					ImGui::Text("Bilateral sigma");
					ImGui::Columns(2, "##cacao_bilateral", false);
					{
						ImGui::Text("Blur term");
						GUI::InputClamp(0.0f, FLT_MAX, ssaoSettings.cacao.bilateralSigmaSquared,
							ImGui::InputFloat("##cacao_bil_sigma", &ssaoSettings.cacao.bilateralSigmaSquared, 0.1f, 1.0f, "%.1f"));
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Sigma squared value for use in bilateral upsampler giving Gaussian blur term");
					}
					ImGui::NextColumn();
					{
						ImGui::Text("Similarity weight");
						GUI::InputClamp(0.0f, FLT_MAX, ssaoSettings.cacao.bilateralSimilarityDistanceSigma,
							ImGui::InputFloat("##cacao_bil_similarity", &ssaoSettings.cacao.bilateralSimilarityDistanceSigma, 0.01f, 0.1f, "%.2f"));
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Sigma squared value for use in bilateral upsampler giving similarity weighting for neighbouring pixels");
					}
					ImGui::Columns(1);
					ImGui::NewLine();
				}
			}
			break;
		}
		}
		switch (Settings::GetUpscaler())
		{
		default:
			ZE_ENUM_UNHANDLED();
		case UpscalerType::None:
		case UpscalerType::XeSS: // At the moment no options for XeSS since only quality mode can be chosen
			break;
		case UpscalerType::Fsr1:
		case UpscalerType::Fsr2:
		case UpscalerType::NIS:
		{
			const bool fsr2 = Settings::GetUpscaler() == UpscalerType::Fsr2;
			const bool nis = Settings::GetUpscaler() == UpscalerType::NIS;
			if (ImGui::CollapsingHeader(nis ? "NIS" : (fsr2 ? "FSR2" : "FSR1")))
			{
				ImGui::Columns(2, "##sharpness_settings", false);
				{
					ImGui::Text("Sharpness");
				}
				ImGui::NextColumn();
				{
					ImGui::Checkbox("##enable_sharpness", &enableSharpening);
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Enable an additional sharpening pass");
				}
				ImGui::Columns(1);

				if (!enableSharpening)
					ImGui::BeginDisabled(true);
				GUI::InputClamp(0.0f, 1.0f, sharpness,
					ImGui::InputFloat("##fsr_sharpness", &sharpness, 0.01f, 0.1f, "%.2f"));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("The sharpness value between 0 and 1, where 0 is no additional sharpness and 1 is maximum additional sharpness");
				if (!enableSharpening)
					ImGui::EndDisabled();
				ImGui::NewLine();
			}
			break;
		}
		}
		if (Settings::IsEnabledSSSR())
		{
			if (ImGui::CollapsingHeader("SSSR"))
			{
				GUI::InputClamp(0.0f, 1.0f, iblFactor,
					ImGui::InputFloat("##ibl_factor", &iblFactor, 0.01f, 0.1f, "%.2f"));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("A factor to control the intensity of the image based lighting. Set to 1 for an HDR probe.");

				ImGui::Text("Temporal Stability");
				ImGui::SetNextItemWidth(-1.0f);
				GUI::InputClamp(0.0f, 1.0f, sssrSettings.TemporalStabilityFactor,
					ImGui::InputFloat("##sssr_temp_stability", &sssrSettings.TemporalStabilityFactor, 0.01f, 0.1f, "%.2f"));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("A factor to control the accmulation of history values. Higher values reduce noise, but are more likely to exhibit ghosting artefacts.");

				ImGui::Text("Depth Buffer Thickness");
				ImGui::SetNextItemWidth(-1.0f);
				GUI::InputClamp(0.0f, 0.03f, sssrSettings.DepthBufferThickness,
					ImGui::InputFloat("##sssr_depth_thicc", &sssrSettings.DepthBufferThickness, 0.001f, 0.01f, "%.3f"));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("A bias for accepting hits. Larger values can cause streaks, lower values can cause holes.");

				ImGui::Text("Roughness Threshold");
				ImGui::SetNextItemWidth(-1.0f);
				GUI::InputClamp(0.0f, 1.0f, sssrSettings.RoughnessThreshold,
					ImGui::InputFloat("##sssr_rough_threshold", &sssrSettings.RoughnessThreshold, 0.01f, 0.1f, "%.2f"));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Regions with a roughness value greater than this threshold won't spawn rays.");

				ImGui::Text("Temporal Variance Threshold");
				ImGui::SetNextItemWidth(-1.0f);
				GUI::InputClamp(0.0f, 0.01f, sssrSettings.VarianceThreshold,
					ImGui::InputFloat("##sssr_variance_threshold", &sssrSettings.VarianceThreshold, 0.0001f, 0.001f, "%.4f"));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Luminance differences between history results will trigger an additional ray if they are greater than this threshold value.");

				ImGui::Text("Max Traversal Iterations");
				ImGui::SetNextItemWidth(-1.0f);
				GUI::InputClamp(0U, 256U, sssrSettings.MaxTraversalIntersections,
					ImGui::InputInt("##sssr_max_intersect", reinterpret_cast<int*>(&sssrSettings.MaxTraversalIntersections), 1, 50));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Caps the maximum number of lookups that are performed from the depth buffer hierarchy. Most rays should terminate after approximately 20 lookups.");

				ImGui::Text("Min Traversal Occupancy");
				ImGui::SetNextItemWidth(-1.0f);
				GUI::InputClamp(0U, 32U, sssrSettings.MinTraversalOccupancy,
					ImGui::InputInt("##sssr_min_occupancy", reinterpret_cast<int*>(&sssrSettings.MinTraversalOccupancy), 1, 10));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Exit the core loop early if less than this number of threads are running.");

				ImGui::Text("Most Detailed Mip");
				ImGui::SetNextItemWidth(-1.0f);
				GUI::InputClamp(0U, 5U, sssrSettings.MostDetailedMip,
					ImGui::InputInt("##sssr_mip_detail", reinterpret_cast<int*>(&sssrSettings.MostDetailedMip), 1, 1));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("The most detailed MIP map level in the depth hierarchy. Perfect mirrors always use 0 as the most detailed level.");

				constexpr std::array<const char*, 3> SAMPLES = { "1", "2", "4" };
				U8 sampleIndex = Utils::SafeCast<U8>(sssrSettings.SamplesPerQuad == 4 ? 2 : sssrSettings.SamplesPerQuad - 1);
				ImGui::SetNextItemWidth(50.0f);
				if (ImGui::BeginCombo("Samples per quad", SAMPLES.at(sampleIndex)))
				{
					for (U8 i = 0; const char* samples : SAMPLES)
					{
						const bool selected = i == sampleIndex;
						if (ImGui::Selectable(samples, selected))
						{
							sampleIndex = i;
							sssrSettings.SamplesPerQuad = sampleIndex == 2 ? 4 : sampleIndex + 1;
						}
						if (selected)
							ImGui::SetItemDefaultFocus();
						++i;
					}
					ImGui::EndCombo();
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("The minimum number of rays per quad. Variance guided tracing can increase this up to a maximum of 4.");

				ImGui::Checkbox("Enable Variance Guided Tracing##sssr_enable_variance", &sssrSettings.TemporalVarianceGuidedTracingEnabled);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("A boolean controlling whether a ray should be spawned on pixels where a temporal variance is detected or not.");
				ImGui::NewLine();
			}
		}
		// If any settings data updated then upload new buffer
		if (change)
			execData.SettingsBuffer.Update(dev, assets.GetDisk(), { INVALID_EID, &settingsData, nullptr, sizeof(DataPBR) });
	}
}