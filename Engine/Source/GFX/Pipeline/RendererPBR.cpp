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
		switch (ssaoSettings.xegtao.QualityLevel)
		{
		default:
			ZE_FAIL("Unknown XeGTAO quality level!");
			[[fallthrough]];
		case 0: // Low
		{
			settingsData.XeGTAOSliceCount = 1.0f;
			settingsData.XeGTAOStepsPerSlice = 2.0f;
			break;
		}
		case 1: // Medium
		{
			settingsData.XeGTAOSliceCount = 2.0f;
			settingsData.XeGTAOStepsPerSlice = 2.0f;
			break;
		}
		case 2: // High
		{
			settingsData.XeGTAOSliceCount = 3.0f;
			settingsData.XeGTAOStepsPerSlice = 3.0f;
			break;
		}
		case 3: // Ultra
		{
			settingsData.XeGTAOSliceCount = 9.0f;
			settingsData.XeGTAOStepsPerSlice = 3.0f;
			break;
		}
		}
	}

	constexpr void RendererPBR::SetupSSAOData(U32 width, U32 height) noexcept
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
			ssaoSettings.xegtao.DenoisePasses = 1;
			settingsData.XeGTAOData.ViewportSize = { Utils::SafeCast<int>(width), Utils::SafeCast<int>(height) };
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

	void RendererPBR::Init(Device& dev, CommandList& mainList, U32 width, U32 height, const ParamsPBR& params)
	{
		const U32 outlineBuffWidth = width / 2;
		const U32 outlineBuffHeight = height / 2;
		FrameBufferDesc frameBufferDesc;
		frameBufferDesc.Init(11, width, height);

#pragma region Framebuffer definition
		RID gbuffNormalCompute = 0;
		RID gbuffDepthCompute = 0;
		if (Settings::GetAOType() != AOType::None)
		{
			gbuffNormalCompute = frameBufferDesc.AddResource(
				{ width, height, 1, FrameResourceFlags::None, PixelFormat::R16G16_Float, ColorF4() });
			gbuffDepthCompute = frameBufferDesc.AddResource(
				{ width, height, 1, FrameResourceFlags::ForceDSV, PixelFormat::DepthOnly, ColorF4(), 0.0f, 0 });
		}
		const RID gbuffColor = frameBufferDesc.AddResource(
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::R8G8B8A8_UNorm, ColorF4() });
		const RID gbuffNormal = frameBufferDesc.AddResource(
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::R16G16_Float, ColorF4() });
		const RID gbuffSpecular = frameBufferDesc.AddResource(
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::R16G16B16A16_Float, ColorF4() });
		const RID gbuffDepth = frameBufferDesc.AddResource(
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::DepthOnly, ColorF4(), 0.0f, 0 });
		const RID lightbuffColor = frameBufferDesc.AddResource(
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::R16G16B16A16_Float, ColorF4(0.0f, 0.0f, 0.0f, 0.0f) });
		const RID lightbuffSpecular = frameBufferDesc.AddResource(
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::R16G16B16A16_Float, ColorF4(0.0f, 0.0f, 0.0f, 0.0f) });
		RID ssao = 0;
		if (Settings::GetAOType() != AOType::None)
			ssao = frameBufferDesc.AddResource({ width, height, 1, FrameResourceFlags::ForceSRV, PixelFormat::R8_UInt, ColorF4() });
		const RID rawScene = frameBufferDesc.AddResource(
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::R16G16B16A16_Float, ColorF4() });
		const RID outline = frameBufferDesc.AddResource(
			{ outlineBuffWidth, outlineBuffHeight, 1, FrameResourceFlags::None, Settings::BackbufferFormat, ColorF4(0.0f, 0.0f, 0.0f, 0.0f) });
		const RID outlineBlur = frameBufferDesc.AddResource(
			{ outlineBuffWidth, outlineBuffHeight, 1, FrameResourceFlags::None, Settings::BackbufferFormat, ColorF4(0.0f, 0.0f, 0.0f, 0.0f) });
		const RID outlineDepth = frameBufferDesc.AddResource(
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::DepthStencil, ColorF4(), 1.0f, 0 }); // TODO: Inverse depth
#pragma endregion

		std::vector<GFX::Pipeline::RenderNode> nodes;
		RendererBuildData buildData = { execData.Bindings, execData.Assets.GetSchemaLibrary() };
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
		SetupBlurData(outlineBuffWidth, outlineBuffHeight);
		SetupSSAOData(width, height);

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
				{ width, height, 1, FrameResourceFlags::ForceSRV, PixelFormat::R32_Float, ColorF4(), 0.0f, 0, 5 });
			node.AddInnerBuffer(Resource::StateUnorderedAccess,
				{ width, height, 1, FrameResourceFlags::ForceSRV, frameBufferDesc.GetFormat(ssao), ColorF4() });
			node.AddInnerBuffer(Resource::StateUnorderedAccess,
				{ width, height, 1, FrameResourceFlags::ForceSRV, PixelFormat::R8_UNorm, ColorF4() });
			node.AddOutput("SB", Resource::StateUnorderedAccess, ssao);
			nodes.emplace_back(std::move(node));
			break;
		}
		case AOType::CACAO:
		{
			ZE_MAKE_NODE("ssao", QueueType::Compute, CACAO, dev, buildData, width, height);
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
			ZE_MAKE_NODE("hdrGamma", QueueType::Main, HDRGammaCorrection, dev, buildData, Settings::BackbufferFormat);
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
		if (Settings::GetAOType() == AOType::XeGTAO)
		{
			XeGTAO::GTAOUpdateConstants(settingsData.XeGTAOData,
				settingsData.XeGTAOData.ViewportSize.x,
				settingsData.XeGTAOData.ViewportSize.y,
				ssaoSettings.xegtao, reinterpret_cast<const float*>(&currentProjection), true, 0);
		}
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
		dynamicData.View = Math::XMMatrixLookToLH(Math::XMLoadFloat3(&dynamicData.CameraPos),
			Math::XMLoadFloat3(&currentCamera.EyeDirection),
			Math::XMLoadFloat3(&currentCamera.UpVector));
		dynamicData.ViewProjection = dynamicData.View * Math::XMLoadFloat4x4(&currentProjection);

		dynamicData.View = Math::XMMatrixTranspose(dynamicData.View);
		dynamicData.ViewProjectionInverse = Math::XMMatrixTranspose(Math::XMMatrixInverse(nullptr, dynamicData.ViewProjection));
		dynamicData.ViewProjection = Math::XMMatrixTranspose(dynamicData.ViewProjection);

		if (Settings::GetAOType() == AOType::CACAO)
		{
			ssaoSettings.cacao.temporalSupersamplingAngleOffset = Math::PI * Utils::SafeCast<float>(Settings::GetFrameIndex() % 3) / 3.0f;
			ssaoSettings.cacao.temporalSupersamplingRadiusOffset = 1.0f + 0.1f * (Utils::SafeCast<float>(Settings::GetFrameIndex() % 3) - 1.0f) / 3.0f;
		}
	}

	void RendererPBR::ShowWindow(Device& dev)
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
				const int quality = ssaoSettings.xegtao.QualityLevel;
				const int denoise = ssaoSettings.xegtao.DenoisePasses;
				change |= XeGTAO::GTAOImGuiSettings(ssaoSettings.xegtao);
				change |= quality != ssaoSettings.xegtao.QualityLevel || denoise != ssaoSettings.xegtao.DenoisePasses;
				if (change)
				{
					SetupXeGTAOQuality();
					XeGTAO::GTAOUpdateConstants(settingsData.XeGTAOData,
						settingsData.XeGTAOData.ViewportSize.x,
						settingsData.XeGTAOData.ViewportSize.y,
						ssaoSettings.xegtao, reinterpret_cast<const float*>(&currentProjection), true, 0);
				}
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
				}
			}
			break;
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