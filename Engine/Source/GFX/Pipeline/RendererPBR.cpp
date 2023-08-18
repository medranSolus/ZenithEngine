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

	constexpr void RendererPBR::SetupSsaoQuality() noexcept
	{
		switch (ssaoSettings.QualityLevel)
		{
		default:
			ZE_FAIL("Unknown SSAO quality level!");
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

	constexpr void RendererPBR::SetupSsaoData(U32 width, U32 height) noexcept
	{
		ssaoSettings.DenoisePasses = 1;
		settingsData.SsaoData.ViewportSize = { Utils::SafeCast<int>(width), Utils::SafeCast<int>(height) };
		SetupSsaoQuality();
	}

	void RendererPBR::Init(Device& dev, CommandList& mainList, U32 width, U32 height, const ParamsPBR& params)
	{
		const U32 outlineBuffWidth = width / 2;
		const U32 outlineBuffHeight = height / 2;
		FrameBufferDesc frameBufferDesc;
		frameBufferDesc.Init(11, width, height);

#pragma region Framebuffer definition
		const RID gbuffNormalCompute = frameBufferDesc.AddResource(
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::R16G16_Float, ColorF4() });
		const RID gbuffDepthCompute = frameBufferDesc.AddResource(
			{ width, height, 1, FrameResourceFlags::ForceDSV, PixelFormat::DepthOnly, ColorF4(), 1.0f, 0 }); // TODO: Inverse depth
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
		RendererBuildData buildData = { execData.Bindings, execData.Assets.GetSchemaLibrary() };
		SetupRenderSlots(buildData);

		blurSigma = params.Sigma;
		settingsData.Gamma = params.Gamma;
		settingsData.GammaInverse = 1.0f / params.Gamma;
		settingsData.AmbientLight = { 0.05f, 0.05f, 0.05f };
		settingsData.HDRExposure = params.HDRExposure;
		settingsData.ShadowMapSize = Utils::SafeCast<float>(params.ShadowMapSize);
		settingsData.ShadowBias = Utils::SafeCast<float>(params.ShadowBias) / settingsData.ShadowMapSize;
		settingsData.ShadowNormalOffset = params.ShadowNormalOffset;
		SetupBlurData(outlineBuffWidth, outlineBuffHeight);
		SetupSsaoData(width, height);

		dev.BeginUploadRegion();
		execData.SettingsBuffer.Init(dev, &settingsData, sizeof(DataPBR));
		dev.StartUpload();

#pragma region Geometry
		{
			ZE_MAKE_NODE("lambertian", QueueType::Main, Lambertian, dev, buildData,
				frameBufferDesc.GetFormat(gbuffDepth), frameBufferDesc.GetFormat(gbuffColor),
				frameBufferDesc.GetFormat(gbuffNormal), frameBufferDesc.GetFormat(gbuffSpecular));
			node.AddOutput("DS", Resource::StateDepthWrite, gbuffDepth);
			node.AddOutput("GB_C", Resource::StateRenderTarget, gbuffColor);
			node.AddOutput("GB_N", Resource::StateRenderTarget, gbuffNormal);
			node.AddOutput("GB_S", Resource::StateRenderTarget, gbuffSpecular);
			nodes.emplace_back(std::move(node));
		}
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
				{ params.ShadowMapSize, params.ShadowMapSize, 1, FrameResourceFlags::None, PixelFormat::DepthOnly, ColorF4(), 1.0f, 0 });
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
				{ params.ShadowMapSize, params.ShadowMapSize, 1, FrameResourceFlags::None, PixelFormat::DepthOnly, ColorF4(), 1.0f, 0 });
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
				{ params.ShadowMapSize, params.ShadowMapSize, 1, FrameResourceFlags::Cube, PixelFormat::DepthOnly, ColorF4(), 1.0f, 0 });
			node.AddOutput("LB_C", Resource::StateRenderTarget, lightbuffColor);
			node.AddOutput("LB_S", Resource::StateRenderTarget, lightbuffSpecular);
			nodes.emplace_back(std::move(node));
		}
		{
			ZE_MAKE_NODE("ssao", QueueType::Compute, SSAO, dev, buildData);
			node.AddInput("lambertianComputeCopy.DS", Resource::StateShaderResourceNonPS);
			node.AddInput("lambertianComputeCopy.GB_N", Resource::StateShaderResourceNonPS);
			node.AddInnerBuffer(Resource::StateUnorderedAccess,
				{ width, height, 1, FrameResourceFlags::ForceSRV, PixelFormat::R32_Float, ColorF4(), 0.0f, 0, 5 });
			node.AddInnerBuffer(Resource::StateUnorderedAccess,
				{ width, height, 1, FrameResourceFlags::ForceSRV, frameBufferDesc.GetFormat(ssao), ColorF4() });
			node.AddInnerBuffer(Resource::StateUnorderedAccess,
				{ width, height, 1, FrameResourceFlags::ForceSRV, PixelFormat::R8_UNorm, ColorF4() });
			node.AddOutput("SB", Resource::StateUnorderedAccess, ssao);
			nodes.emplace_back(std::move(node));
		}
		{
			ZE_MAKE_NODE("lightCombine", QueueType::Main, LightCombine, dev, buildData, frameBufferDesc.GetFormat(rawScene));
			node.AddInput("ssao.SB", Resource::StateShaderResourcePS);
			node.AddInput("lambertian.GB_C", Resource::StateShaderResourcePS);
			node.AddInput("pointLight.LB_C", Resource::StateShaderResourcePS);
			node.AddInput("pointLight.LB_S", Resource::StateShaderResourcePS);
			node.AddOutput("RT", Resource::StateRenderTarget, rawScene);
			nodes.emplace_back(std::move(node));
		}
#pragma endregion
#pragma region Geometry effects
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
			ZE_MAKE_NODE("verticalBlur", QueueType::Main, VerticalBlur, dev, buildData, frameBufferDesc.GetFormat(rawScene), frameBufferDesc.GetFormat(outlineDepth));
			node.AddInput("horizontalBlur.RT", Resource::StateShaderResourcePS);
			node.AddInput("skybox.RT", Resource::StateRenderTarget);
			node.AddInput("outlineDraw.DS", Resource::StateDepthRead);
			node.AddOutput("RT", Resource::StateRenderTarget, rawScene);
			nodes.emplace_back(std::move(node));
		}
		{
			ZE_MAKE_NODE("wireframe", QueueType::Main, Wireframe, dev, buildData,
				frameBufferDesc.GetFormat(rawScene), frameBufferDesc.GetFormat(gbuffDepth));
			node.AddInput("verticalBlur.RT", Resource::StateRenderTarget);
			node.AddInput("skybox.DS", Resource::StateDepthWrite);
			node.AddOutput("RT", Resource::StateRenderTarget, rawScene);
			nodes.emplace_back(std::move(node));
		}
#pragma endregion
#pragma region Post processing
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
			ZE_MAKE_NODE("hdrGamma", QueueType::Main, HDRGammaCorrection, dev, buildData, Settings::GetBackbufferFormat());
			node.AddInput("wireframe.RT", Resource::StateShaderResourcePS);
			node.AddOutput("RT", Resource::StateRenderTarget, BACKBUFFER_RID);
			nodes.emplace_back(std::move(node));
		}
#pragma endregion
		Finalize(dev, mainList, nodes, frameBufferDesc, buildData, params.MinimizeRenderPassDistances);
		dev.EndUploadRegion();
	}

	void RendererPBR::UpdateSettingsData(Device& dev, const Matrix& projection)
	{
		Math::XMStoreFloat4x4(&currentProjection, projection);
		XeGTAO::GTAOUpdateConstants(settingsData.SsaoData,
			settingsData.SsaoData.ViewportSize.x,
			settingsData.SsaoData.ViewportSize.y,
			ssaoSettings, reinterpret_cast<const float*>(&currentProjection), true, 0);
		dev.BeginUploadRegion();
		execData.SettingsBuffer.Update(dev, &settingsData, sizeof(DataPBR));
		dev.StartUpload();
		dev.EndUploadRegion();
	}

	void RendererPBR::UpdateWorldData(Device& dev, EID camera) noexcept
	{
		ZE_ASSERT((GetRegistry().all_of<Data::TransformGlobal, Data::Camera>(camera)),
			"Current camera does not have all required components!");

		const auto& transform = GetRegistry().get<Data::Transform>(camera); // TODO: Change into TransformGlobal later
		cameraRotation = transform.Rotation;

		// Setup shader world data
		dynamicData.CameraPos = transform.Position;
		const auto& currentCamera = GetRegistry().get<Data::Camera>(camera);
		dynamicData.NearClip = currentCamera.Projection.NearClip;
		dynamicData.FarClip = currentCamera.Projection.FarClip;
		dynamicData.View = Math::XMMatrixLookToLH(Math::XMLoadFloat3(&dynamicData.CameraPos),
			Math::XMLoadFloat3(&currentCamera.EyeDirection),
			Math::XMLoadFloat3(&currentCamera.UpVector));
		dynamicData.ViewProjection = dynamicData.View * Math::XMLoadFloat4x4(&currentProjection);

		dynamicData.View = Math::XMMatrixTranspose(dynamicData.View);
		dynamicData.ViewProjectionInverse = Math::XMMatrixTranspose(Math::XMMatrixInverse(nullptr, dynamicData.ViewProjection));
		dynamicData.ViewProjection = Math::XMMatrixTranspose(dynamicData.ViewProjection);
	}

	void RendererPBR::ShowWindow(Device& dev)
	{
		bool change = false;
		if (ImGui::CollapsingHeader("Outline"))
		{
			ImGui::Columns(2, "##outline_options", false);
			ImGui::Text("Blur radius");
			ImGui::SetNextItemWidth(-1.0f);
			change |= ImGui::SliderInt("##blur_radius", &settingsData.BlurRadius, 1, DataPBR::BLUR_KERNEL_RADIUS);
			ImGui::NextColumn();
			ImGui::Text("Outline range");
			ImGui::SetNextItemWidth(-1.0f);
			if (ImGui::InputFloat("##blur_sigma", &blurSigma, 0.1f, 0.0f, "%.1f"))
			{
				change = true;
				if (blurSigma < 0.1f)
					blurSigma = 0.1f;
				else if (blurSigma > 25.0f)
					blurSigma = 25.0f;
				SetupBlurKernel();
			}
			ImGui::Columns(1);
		}
		if (ImGui::CollapsingHeader("Display"))
		{
			ImGui::Columns(2, "##display_options", false);
			ImGui::Text("Gamma correction");
			ImGui::SetNextItemWidth(-1.0f);
			if (ImGui::InputFloat("##gamma", &settingsData.Gamma, 0.1f, 0.0f, "%.1f"))
			{
				change = true;
				if (settingsData.Gamma < 1.0f)
					settingsData.Gamma = 1.0f;
				else if (settingsData.Gamma > 10.0f)
					settingsData.Gamma = 10.0f;
				settingsData.GammaInverse = 1.0f / settingsData.Gamma;
			}
			ImGui::NextColumn();
			ImGui::Text("HDR exposure");
			ImGui::SetNextItemWidth(-1.0f);
			if (ImGui::InputFloat("##hdr", &settingsData.HDRExposure, 0.1f, 0.0f, "%.1f"))
			{
				change = true;
				if (settingsData.HDRExposure < 0.1f)
					settingsData.HDRExposure = 0.1f;
				SetupBlurIntensity();
			}
			ImGui::Columns(1);
		}
		if (ImGui::CollapsingHeader("Shadows"))
		{
			ImGui::Columns(2, "##shadow_options", false);
			ImGui::Text("Depth bias");
			ImGui::SetNextItemWidth(-1.0f);
			S32 bias = Utils::SafeCast<S32>(settingsData.ShadowBias * settingsData.ShadowMapSize);
			if (ImGui::InputInt("##depth_bias", &bias))
			{
				change = true;
				settingsData.ShadowBias = Utils::SafeCast<float>(bias) / settingsData.ShadowMapSize;
			}
			ImGui::NextColumn();
			ImGui::Text("Normal offset");
			ImGui::SetNextItemWidth(-1.0f);
			if (ImGui::InputFloat("##normal_offset", &settingsData.ShadowNormalOffset, 0.001f, 0.0f, "%.3f"))
			{
				change = true;
				if (settingsData.ShadowNormalOffset < 0.0f)
					settingsData.ShadowNormalOffset = 0.0f;
				else if (settingsData.ShadowNormalOffset > 1.0f)
					settingsData.ShadowNormalOffset = 1.0f;
			}
			ImGui::Columns(1);
			ImGui::Text("Ambient color");
			ImGui::SetNextItemWidth(-5.0f);
			change |= ImGui::ColorEdit3("##ambient_color", reinterpret_cast<float*>(&settingsData.AmbientLight),
				ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoLabel);
		}
		if (ImGui::CollapsingHeader("SSAO"))
		{
			// GTAOImGuiSettings() don't indicate if quality or denoise passes has been updated...
			const int quality = ssaoSettings.QualityLevel;
			const int denoise = ssaoSettings.DenoisePasses;
			change |= XeGTAO::GTAOImGuiSettings(ssaoSettings);
			change |= quality != ssaoSettings.QualityLevel || denoise != ssaoSettings.DenoisePasses;
			if (change)
			{
				SetupSsaoQuality();
				XeGTAO::GTAOUpdateConstants(settingsData.SsaoData,
					settingsData.SsaoData.ViewportSize.x,
					settingsData.SsaoData.ViewportSize.y,
					ssaoSettings, reinterpret_cast<const float*>(&currentProjection), true, 0);
			}
		}
		// If any settings data updated then upload new buffer
		if (change)
		{
			dev.BeginUploadRegion();
			execData.SettingsBuffer.Update(dev, &settingsData, sizeof(DataPBR));
			dev.StartUpload();
			dev.EndUploadRegion();
		}
	}
}