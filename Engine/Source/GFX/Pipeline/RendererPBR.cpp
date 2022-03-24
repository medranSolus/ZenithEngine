#include "GFX/Pipeline/RendererPBR.h"
#include "GFX/Pipeline/RenderPasses.h"

#define ZE_MAKE_NODE(name, queueType, passNamespace) RenderNode node(name, queueType, RenderPass::##passNamespace##::Execute)
#define ZE_MAKE_NODE_STATIC(name, queueType, passNamespace, ...) RenderNode node(name, queueType, nullptr, nullptr, RenderPass::##passNamespace##::Execute(__VA_ARGS__), true)
#define ZE_MAKE_NODE_DATA(name, queueType, passNamespace, ...) RenderNode node(name, queueType, RenderPass::passNamespace::Execute, RenderPass::passNamespace::Clean, RenderPass::passNamespace::Setup(__VA_ARGS__))

namespace ZE::GFX::Pipeline
{
	void RendererPBR::SetupRenderSlots(RendererBuildData& buildData) noexcept
	{
		buildData.RendererSlots.AddRange(
			{
				1, 13,
				Resource::ShaderType::Pixel | Resource::ShaderType::Compute,
				Binding::RangeFlag::CBV
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

	constexpr void RendererPBR::SetupBlurData(U32 width, U32 height, float sigma) noexcept
	{
		settingsData.BlurRadius = DataPBR::BLUR_KERNEL_RADIUS;
		settingsData.BlurWidth = width;
		settingsData.BlurHeight = height;
		settingsData.BlurIntensity = settingsData.HDRExposure < 1.0f ? 1.0f / settingsData.HDRExposure : 1.0f;

		float sum = 0.0f;
		for (S32 i = 0; i <= settingsData.BlurRadius; ++i)
		{
			const float g = Math::Gauss(static_cast<float>(i), sigma);
			sum += g;
			settingsData.BlurCoefficients[i].x = g;
		}
		for (S32 i = 0; i <= settingsData.BlurRadius; ++i)
			settingsData.BlurCoefficients[i].x /= sum;
	}

	constexpr void RendererPBR::SetupSsaoData(U32 width, U32 height) noexcept
	{
		ssaoSettings.DenoisePasses = 1;
		settingsData.SsaoData.ViewportSize = { static_cast<int>(width), static_cast<int>(height) };
		switch (ssaoSettings.QualityLevel)
		{
		default:
			ZE_ASSERT(false, "Unknown SSAO quality level!");
		case 0: // Low
		{
			settingsData.SsaoSliceCount = 1.0f;
			settingsData.SsaoStepsPerSlice = 2.0f;
			break;
		}
		case 1: // Medium
		{
			settingsData.SsaoSliceCount = 2.0f;
			settingsData.SsaoStepsPerSlice = 2.0f;
			break;
		}
		case 2: // High
		{
			settingsData.SsaoSliceCount = 3.0f;
			settingsData.SsaoStepsPerSlice = 3.0f;
			break;
		}
		case 3: // Ultra
		{
			settingsData.SsaoSliceCount = 9.0f;
			settingsData.SsaoStepsPerSlice = 3.0f;
			break;
		}
		}
	}

	void RendererPBR::Init(Device& dev, CommandList& mainList, Resource::Texture::Library& texLib,
		U32 width, U32 height, const ParamsPBR& params)
	{
		const U32 outlineBuffWidth = width / 2;
		const U32 outlineBuffHeight = height / 2;
		FrameBufferDesc frameBufferDesc;
		frameBufferDesc.Init(11, width, height);

#pragma region Framebuffer definition
		const RID ssao = frameBufferDesc.AddResource(
			{ width, height, 1, FrameResourceFlags::ForceSRV, PixelFormat::R8_UInt, ColorF4() });
		const RID gbuffColor = frameBufferDesc.AddResource(
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::R8G8B8A8_UNorm, ColorF4() });
		const RID gbuffNormal = frameBufferDesc.AddResource(
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::R16G16_Float, ColorF4() });
		const RID gbuffSpecular = frameBufferDesc.AddResource(
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::R16G16B16A16_Float, ColorF4() });
		const RID gbuffDepth = frameBufferDesc.AddResource(
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::DepthOnly, ColorF4(), 1.0f, 0 }); // TODO: Inverse depth
		const RID lightbuffColor = frameBufferDesc.AddResource(
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::R16G16B16A16_Float, ColorF4(0.0f, 0.0f, 0.0f, 0.0f) });
		const RID lightbuffSpecular = frameBufferDesc.AddResource(
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::R16G16B16A16_Float, ColorF4(0.0f, 0.0f, 0.0f, 0.0f) });
		const RID rawScene = frameBufferDesc.AddResource(
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::R16G16B16A16_Float, ColorF4() });
		const RID outline = frameBufferDesc.AddResource(
			{ outlineBuffWidth, outlineBuffHeight, 1, FrameResourceFlags::None, Settings::GetBackbufferFormat(), ColorF4(0.0f, 0.0f, 0.0f, 0.0f) });
		const RID outlineBlur = frameBufferDesc.AddResource(
			{ outlineBuffWidth, outlineBuffHeight, 1, FrameResourceFlags::None, Settings::GetBackbufferFormat(), ColorF4(0.0f, 0.0f, 0.0f, 0.0f) });
		const RID outlineDepth = frameBufferDesc.AddResource(
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::DepthStencil, ColorF4(), 1.0f, 0 }); // TODO: Inverse depth
#pragma endregion

		std::vector<GFX::Pipeline::RenderNode> nodes;
		RendererBuildData buildData = { execData.Bindings, texLib };
		SetupRenderSlots(buildData);

		settingsData.Gamma = params.Gamma;
		settingsData.GammaInverse = 1.0f / params.Gamma;
		settingsData.AmbientLight = { 0.05f, 0.05f, 0.05f };
		settingsData.HDRExposure = params.HDRExposure;
		settingsData.ShadowMapSize = static_cast<float>(params.ShadowMapSize);
		settingsData.ShadowBias = static_cast<float>(params.ShadowBias) / settingsData.ShadowMapSize;
		settingsData.ShadowNormalOffset = params.ShadowNormalOffset;
		SetupBlurData(outlineBuffWidth, outlineBuffHeight, params.Sigma);
		SetupSsaoData(width, height);

		dev.BeginUploadRegion();
		execData.SettingsBuffer.Init(dev, &settingsData, sizeof(DataPBR), false);
		dev.StartUpload();

#pragma region Geometry
		{
			ZE_MAKE_NODE_DATA("lambertian", QueueType::Main, Lambertian, dev, buildData,
				frameBufferDesc.GetFormat(gbuffDepth), frameBufferDesc.GetFormat(gbuffColor),
				frameBufferDesc.GetFormat(gbuffNormal), frameBufferDesc.GetFormat(gbuffSpecular));
			node.AddOutput("DS", Resource::State::DepthWrite, gbuffDepth);
			node.AddOutput("GB_C", Resource::State::RenderTarget, gbuffColor);
			node.AddOutput("GB_N", Resource::State::RenderTarget, gbuffNormal);
			node.AddOutput("GB_S", Resource::State::RenderTarget, gbuffSpecular);
			nodes.emplace_back(std::move(node));
		}
#pragma endregion
#pragma region Lightning
		{
			ZE_MAKE_NODE_DATA("dirLight", QueueType::Main, DirectionalLight, dev, buildData,
				frameBufferDesc.GetFormat(lightbuffColor), frameBufferDesc.GetFormat(lightbuffSpecular),
				PixelFormat::R32_Float, PixelFormat::DepthOnly);
			node.AddInput("lambertian.GB_N", Resource::State::ShaderResourcePS);
			node.AddInput("lambertian.GB_S", Resource::State::ShaderResourcePS);
			node.AddInput("lambertian.DS", Resource::State::ShaderResourcePS);
			node.AddInnerBuffer(Resource::State::RenderTarget,
				{ params.ShadowMapSize, params.ShadowMapSize, 1, FrameResourceFlags::ForceSRV, PixelFormat::R32_Float, { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX } });
			node.AddInnerBuffer(Resource::State::DepthWrite,
				{ params.ShadowMapSize, params.ShadowMapSize, 1, FrameResourceFlags::None, PixelFormat::DepthOnly, ColorF4(), 1.0f, 0 });
			node.AddOutput("LB_C", Resource::State::RenderTarget, lightbuffColor);
			node.AddOutput("LB_S", Resource::State::RenderTarget, lightbuffSpecular);
			nodes.emplace_back(std::move(node));
		}
		{
			ZE_MAKE_NODE_DATA("spotLight", QueueType::Main, SpotLight, dev, buildData,
				frameBufferDesc.GetFormat(lightbuffColor), frameBufferDesc.GetFormat(lightbuffSpecular),
				PixelFormat::R32_Float, PixelFormat::DepthOnly);
			node.AddInput("lambertian.GB_N", Resource::State::ShaderResourcePS);
			node.AddInput("lambertian.GB_S", Resource::State::ShaderResourcePS);
			node.AddInput("lambertian.DS", Resource::State::ShaderResourcePS);
			node.AddInput("dirLight.LB_C", Resource::State::RenderTarget);
			node.AddInput("dirLight.LB_S", Resource::State::RenderTarget);
			node.AddInnerBuffer(Resource::State::RenderTarget,
				{ params.ShadowMapSize, params.ShadowMapSize, 1, FrameResourceFlags::ForceSRV, PixelFormat::R32_Float, { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX } });
			node.AddInnerBuffer(Resource::State::DepthWrite,
				{ params.ShadowMapSize, params.ShadowMapSize, 1, FrameResourceFlags::None, PixelFormat::DepthOnly, ColorF4(), 1.0f, 0 });
			node.AddOutput("LB_C", Resource::State::RenderTarget, lightbuffColor);
			node.AddOutput("LB_S", Resource::State::RenderTarget, lightbuffSpecular);
			nodes.emplace_back(std::move(node));
		}
		{
			ZE_MAKE_NODE_DATA("pointLight", QueueType::Main, PointLight, dev, buildData,
				frameBufferDesc.GetFormat(lightbuffColor), frameBufferDesc.GetFormat(lightbuffSpecular),
				PixelFormat::R32_Float, PixelFormat::DepthOnly);
			node.AddInput("lambertian.GB_N", Resource::State::ShaderResourcePS);
			node.AddInput("lambertian.GB_S", Resource::State::ShaderResourcePS);
			node.AddInput("lambertian.DS", Resource::State::ShaderResourcePS);
			node.AddInput("spotLight.LB_C", Resource::State::RenderTarget);
			node.AddInput("spotLight.LB_S", Resource::State::RenderTarget);
			node.AddInnerBuffer(Resource::State::RenderTarget,
				{ params.ShadowMapSize, params.ShadowMapSize, 1, FrameResourceFlags::Cube | FrameResourceFlags::ForceSRV, PixelFormat::R32_Float, { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX } });
			node.AddInnerBuffer(Resource::State::DepthWrite,
				{ params.ShadowMapSize, params.ShadowMapSize, 1, FrameResourceFlags::Cube, PixelFormat::DepthOnly, ColorF4(), 1.0f, 0 });
			node.AddOutput("LB_C", Resource::State::RenderTarget, lightbuffColor);
			node.AddOutput("LB_S", Resource::State::RenderTarget, lightbuffSpecular);
			nodes.emplace_back(std::move(node));
		}
		{
			ZE_MAKE_NODE_DATA("ssao", QueueType::Compute, SSAO, dev, buildData);
			node.AddInput("lambertian.DS", Resource::State::ShaderResourceNonPS);
			node.AddInput("lambertian.GB_N", Resource::State::ShaderResourceNonPS);
			node.AddInnerBuffer(Resource::State::UnorderedAccess,
				{ width, height, 1, FrameResourceFlags::ForceSRV, PixelFormat::R32_Float, ColorF4(), 0.0f, 0, 5 });
			node.AddInnerBuffer(Resource::State::UnorderedAccess,
				{ width, height, 1, FrameResourceFlags::ForceSRV, PixelFormat::R8_UInt, ColorF4() });
			node.AddInnerBuffer(Resource::State::UnorderedAccess,
				{ width, height, 1, FrameResourceFlags::ForceSRV, PixelFormat::R8_UNorm, ColorF4() });
			node.AddOutput("SB", Resource::State::UnorderedAccess, ssao);
			nodes.emplace_back(std::move(node));
		}
		{
			ZE_MAKE_NODE_DATA("lightCombine", QueueType::Main, LightCombine, dev, buildData, frameBufferDesc.GetFormat(rawScene));
			node.AddInput("ssao.SB", Resource::State::ShaderResourcePS);
			node.AddInput("lambertian.GB_C", Resource::State::ShaderResourcePS);
			node.AddInput("pointLight.LB_C", Resource::State::ShaderResourcePS);
			node.AddInput("pointLight.LB_S", Resource::State::ShaderResourcePS);
			node.AddOutput("RT", Resource::State::RenderTarget, rawScene);
			nodes.emplace_back(std::move(node));
		}
#pragma endregion
#pragma region Geometry effects
		{
			ZE_MAKE_NODE_DATA("outlineDraw", QueueType::Main, OutlineDraw, dev, buildData,
				frameBufferDesc.GetFormat(outline), frameBufferDesc.GetFormat(outlineDepth));
			node.AddOutput("RT", Resource::State::RenderTarget, outline);
			node.AddOutput("DS", Resource::State::DepthWrite, outlineDepth);
			nodes.emplace_back(std::move(node));
		}
		{
			ZE_MAKE_NODE_DATA("horizontalBlur", QueueType::Main, HorizontalBlur, dev, buildData, frameBufferDesc.GetFormat(outlineBlur));
			node.AddInput("outlineDraw.RT", Resource::State::ShaderResourcePS);
			node.AddOutput("RT", Resource::State::RenderTarget, outlineBlur);
			nodes.emplace_back(std::move(node));
		}
		{
			ZE_MAKE_NODE_DATA("verticalBlur", QueueType::Main, VerticalBlur, dev, buildData, frameBufferDesc.GetFormat(rawScene), frameBufferDesc.GetFormat(outlineDepth));
			node.AddInput("horizontalBlur.RT", Resource::State::ShaderResourcePS);
			node.AddInput("skybox.RT", Resource::State::RenderTarget);
			node.AddInput("outlineDraw.DS", Resource::State::DepthRead);
			node.AddOutput("RT", Resource::State::RenderTarget, rawScene);
			nodes.emplace_back(std::move(node));
		}
		{
			ZE_MAKE_NODE_DATA("wireframe", QueueType::Main, Wireframe, dev, buildData,
				frameBufferDesc.GetFormat(rawScene), frameBufferDesc.GetFormat(gbuffDepth));
			node.AddInput("verticalBlur.RT", Resource::State::RenderTarget);
			node.AddInput("skybox.DS", Resource::State::DepthWrite);
			node.AddOutput("RT", Resource::State::RenderTarget, rawScene);
			nodes.emplace_back(std::move(node));
		}
#pragma endregion
#pragma region Post processing
		{
			ZE_MAKE_NODE_DATA("skybox", QueueType::Main, Skybox, dev, buildData,
				frameBufferDesc.GetFormat(rawScene), frameBufferDesc.GetFormat(gbuffDepth),
				params.SkyboxPath, params.SkyboxExt);
			node.AddInput("lightCombine.RT", Resource::State::RenderTarget);
			node.AddInput("lambertian.DS", Resource::State::DepthRead);
			node.AddOutput("RT", Resource::State::RenderTarget, rawScene);
			node.AddOutput("DS", Resource::State::DepthRead, gbuffDepth);
			nodes.emplace_back(std::move(node));
		}
		{
			ZE_MAKE_NODE_DATA("hdrGamma", QueueType::Main, HDRGammaCorrection, dev, buildData, Settings::GetBackbufferFormat());
			node.AddInput("wireframe.RT", Resource::State::ShaderResourcePS);
			node.AddOutput("RT", Resource::State::RenderTarget, BACKBUFFER_RID);
			nodes.emplace_back(std::move(node));
		}
#pragma endregion
		Finalize(dev, mainList, nodes, frameBufferDesc, buildData, params.MinimizeRenderPassDistances);

		execData.DynamicBuffer.Init(dev, nullptr, sizeof(CameraPBR), true);
		dev.EndUploadRegion();
	}

	void RendererPBR::UpdateSettingsData(Device& dev, const Float4x4& projection)
	{
		XeGTAO::GTAOUpdateConstants(settingsData.SsaoData,
			settingsData.SsaoData.ViewportSize.x,
			settingsData.SsaoData.ViewportSize.y,
			ssaoSettings, reinterpret_cast<const float*>(&projection), true, 0);
		dev.BeginUploadRegion();
		execData.SettingsBuffer.Update(dev, &settingsData, sizeof(DataPBR));
		dev.StartUpload();
		dev.EndUploadRegion();
	}

	void RendererPBR::UpdateWorldData(Device& dev, EID camera, const Float4x4& projection) noexcept
	{
		ZE_ASSERT((GetRegistry().all_of<Data::Transform, Data::Camera>(camera)),
			"Current camera does not have all required components!");

		// Setup shader world data
		dynamicData.CameraPos = GetRegistry().get<Data::Transform>(camera).Position;
		const auto& currentCamera = GetRegistry().get<Data::Camera>(camera);
		dynamicData.NearClip = currentCamera.Projection.NearClip;
		dynamicData.FarClip = currentCamera.Projection.FarClip;
		dynamicData.View = Math::XMMatrixLookToLH(Math::XMLoadFloat3(&dynamicData.CameraPos),
			Math::XMLoadFloat3(&currentCamera.EyeDirection),
			Math::XMLoadFloat3(&currentCamera.UpVector));
		dynamicData.ViewProjection = dynamicData.View * Math::XMLoadFloat4x4(&projection);

		dynamicData.View = Math::XMMatrixTranspose(dynamicData.View);
		dynamicData.ViewProjectionInverse = Math::XMMatrixTranspose(Math::XMMatrixInverse(nullptr, dynamicData.ViewProjection));
		dynamicData.ViewProjection = Math::XMMatrixTranspose(dynamicData.ViewProjection);
	}
}