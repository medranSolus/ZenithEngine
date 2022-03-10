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
			Resource::Texture::AddressMode::Repeat,
			Resource::Texture::AddressMode::Mirror
		};
		constexpr Resource::SamplerType TYPES[]
		{
			Resource::SamplerType::Anisotropic,
			Resource::SamplerType::Linear,
			Resource::SamplerType::Point
		};

		buildData.RendererSlots.Samplers.reserve(sizeof(TYPES) * sizeof(ADDRESS_MODES) + 1);
		buildData.RendererSlots.AddSampler(
			{
				Resource::SamplerType::Anisotropic,
				{
					Resource::Texture::AddressMode::Edge,
					Resource::Texture::AddressMode::Edge,
					Resource::Texture::AddressMode::Edge
				},
				0.0f, 4, Resource::CompareMethod::Never,
				Resource::Texture::EdgeColor::TransparentBlack,
				0.0f, FLT_MAX, 0
			});

		U32 slot = 1;
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

	void RendererPBR::SetupSsaoData(U32 width, U32 height) noexcept
	{
		settingsData.SsaoNoiseDimmensions = { width / RenderPass::SSAO::NOISE_WIDTH, height / RenderPass::SSAO::NOISE_HEIGHT };
		settingsData.SsaoBias = 0.188f;
		settingsData.SsaoSampleRadius = 0.69f;
		settingsData.SsaoPower = 2.77f;
		settingsData.SsaoKernelSize = DataPBR::SSAO_KERNEL_MAX_SIZE;

		std::mt19937_64 engine(std::random_device{}());
		for (U32 i = 0; i < DataPBR::SSAO_KERNEL_MAX_SIZE; ++i)
		{
			const Vector sample = Math::XMVectorSet(Math::RandNDC(engine),
				Math::RandNDC(engine), Math::Rand01(engine), 0.0f);

			float scale = static_cast<float>(i) / DataPBR::SSAO_KERNEL_MAX_SIZE;
			scale = Math::Lerp(0.1f, 1.0f, scale * scale);

			Math::XMStoreFloat4(&settingsData.SsaoKernel[i],
				Math::XMVectorMultiply(Math::XMVector3Normalize(sample), Math::XMVectorSet(scale, scale, scale, 0.0f)));
		}
	}

	RendererPBR::~RendererPBR()
	{
		if (worldData.Meshes)
			Table::Clear(worldData.MeshInfo, worldData.Meshes);
		if (worldData.Outlines)
			Table::Clear(worldData.OutlineInfo, worldData.Outlines);
		if (worldData.Wireframes)
			Table::Clear(worldData.WireframeInfo, worldData.Wireframes);
		if (worldData.DirectionalLights)
			Table::Clear(worldData.DirectionalLightInfo, worldData.DirectionalLights);
		if (worldData.SpotLights)
			Table::Clear(worldData.SpotLightInfo, worldData.SpotLights);
		if (worldData.PointLights)
			Table::Clear(worldData.PointLightInfo, worldData.PointLights);
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
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::R32_Float, ColorF4() });
		const RID gbuffColor = frameBufferDesc.AddResource(
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::R8G8B8A8_UNorm, ColorF4() });
		const RID gbuffNormal = frameBufferDesc.AddResource(
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::R32G32_Float, ColorF4() });
		const RID gbuffSpecular = frameBufferDesc.AddResource(
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::R16G16B16A16_Float, ColorF4() });
		const RID gbuffDepth = frameBufferDesc.AddResource(
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::DepthOnly, ColorF4(), 1.0f, 0 }); // TODO: Inverse depth
		const RID lightbuffColor = frameBufferDesc.AddResource(
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::R16G16B16A16_Float, ColorF4() });
		const RID lightbuffSpecular = frameBufferDesc.AddResource(
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::R16G16B16A16_Float, ColorF4() });
		const RID rawScene = frameBufferDesc.AddResource(
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::R16G16B16A16_Float, ColorF4() });
		const RID outline = frameBufferDesc.AddResource(
			{ outlineBuffWidth, outlineBuffHeight, 1, FrameResourceFlags::None, Settings::GetBackbufferFormat(), ColorF4() });
		const RID outlineBlur = frameBufferDesc.AddResource(
			{ outlineBuffWidth, outlineBuffHeight, 1, FrameResourceFlags::None, Settings::GetBackbufferFormat(), ColorF4() });
		const RID outlineDepth = frameBufferDesc.AddResource(
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::DepthStencil, ColorF4(), 1.0f, 0 }); // TODO: Inverse depth
#pragma endregion

		std::vector<GFX::Pipeline::RenderNode> nodes;
		RendererBuildData buildData = { bindings, texLib };
		SetupRenderSlots(buildData);

		settingsData.Gamma = params.Gamma;
		settingsData.GammaInverse = 1.0f / params.Gamma;
		settingsData.AmbientLight = { 0.05f, 0.05f, 0.05f };
		settingsData.HDRExposure = params.HDRExposure;
		settingsData.FrameDimmensions = { width, height };
		settingsData.ShadowMapSize = static_cast<float>(params.ShadowMapSize);
		settingsData.ShadowBias = static_cast<float>(params.ShadowBias) / settingsData.ShadowMapSize;
		settingsData.ShadowNormalOffset = params.ShadowNormalOffset;
		SetupSsaoData(width, height);
		SetupBlurData(outlineBuffWidth, outlineBuffHeight, params.Sigma);

		dev.BeginUploadRegion();
		settingsBuffer.Init(dev, &settingsData, sizeof(DataPBR), false);
		dev.StartUpload();

#pragma region Geometry
		{
			ZE_MAKE_NODE_DATA("lambertian", QueueType::Main, Lambertian, dev, buildData, worldData,
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
			ZE_MAKE_NODE_DATA("dirLight", QueueType::Main, DirectionalLight, dev, buildData, worldData,
				frameBufferDesc.GetFormat(lightbuffColor), frameBufferDesc.GetFormat(lightbuffSpecular),
				PixelFormat::R32_Float, PixelFormat::DepthOnly);
			node.AddInput("lambertian.GB_N", Resource::State::ShaderResourcePS);
			node.AddInput("lambertian.GB_S", Resource::State::ShaderResourcePS);
			node.AddInput("lambertian.DS", Resource::State::ShaderResourcePS);
			node.AddInnerBuffer(Resource::State::RenderTarget,
				{ params.ShadowMapSize, params.ShadowMapSize, 1, FrameResourceFlags::ForceSRV, PixelFormat::R32_Float, ColorF4() });
			node.AddInnerBuffer(Resource::State::DepthWrite,
				{ params.ShadowMapSize, params.ShadowMapSize, 1, FrameResourceFlags::None, PixelFormat::DepthOnly, ColorF4(), 1.0f, 0 });
			node.AddOutput("LB_C", Resource::State::RenderTarget, lightbuffColor);
			node.AddOutput("LB_S", Resource::State::RenderTarget, lightbuffSpecular);
			nodes.emplace_back(std::move(node));
		}
		{
			ZE_MAKE_NODE_DATA("spotLight", QueueType::Main, SpotLight, dev, buildData, worldData,
				frameBufferDesc.GetFormat(lightbuffColor), frameBufferDesc.GetFormat(lightbuffSpecular),
				PixelFormat::R32_Float, PixelFormat::DepthOnly);
			node.AddInput("lambertian.GB_N", Resource::State::ShaderResourcePS);
			node.AddInput("lambertian.GB_S", Resource::State::ShaderResourcePS);
			node.AddInput("lambertian.DS", Resource::State::ShaderResourcePS);
			node.AddInput("dirLight.LB_C", Resource::State::RenderTarget);
			node.AddInput("dirLight.LB_S", Resource::State::RenderTarget);
			node.AddInnerBuffer(Resource::State::RenderTarget,
				{ params.ShadowMapSize, params.ShadowMapSize, 1, FrameResourceFlags::ForceSRV, PixelFormat::R32_Float, ColorF4() });
			node.AddInnerBuffer(Resource::State::DepthWrite,
				{ params.ShadowMapSize, params.ShadowMapSize, 1, FrameResourceFlags::None, PixelFormat::DepthOnly, ColorF4(), 1.0f, 0 });
			node.AddOutput("LB_C", Resource::State::RenderTarget, lightbuffColor);
			node.AddOutput("LB_S", Resource::State::RenderTarget, lightbuffSpecular);
			nodes.emplace_back(std::move(node));
		}
		{
			ZE_MAKE_NODE_DATA("pointLight", QueueType::Main, PointLight, dev, buildData, worldData,
				frameBufferDesc.GetFormat(lightbuffColor), frameBufferDesc.GetFormat(lightbuffSpecular),
				PixelFormat::R32_Float, PixelFormat::DepthOnly);
			node.AddInput("lambertian.GB_N", Resource::State::ShaderResourcePS);
			node.AddInput("lambertian.GB_S", Resource::State::ShaderResourcePS);
			node.AddInput("lambertian.DS", Resource::State::ShaderResourcePS);
			node.AddInput("spotLight.LB_C", Resource::State::RenderTarget);
			node.AddInput("spotLight.LB_S", Resource::State::RenderTarget);
			node.AddInnerBuffer(Resource::State::RenderTarget,
				{ params.ShadowMapSize, params.ShadowMapSize, 1, FrameResourceFlags::Cube | FrameResourceFlags::ForceSRV, PixelFormat::R32_Float, ColorF4() });
			node.AddInnerBuffer(Resource::State::DepthWrite,
				{ params.ShadowMapSize, params.ShadowMapSize, 1, FrameResourceFlags::Cube, PixelFormat::DepthOnly, ColorF4(), 1.0f, 0 });
			node.AddOutput("LB_C", Resource::State::RenderTarget, lightbuffColor);
			node.AddOutput("LB_S", Resource::State::RenderTarget, lightbuffSpecular);
			nodes.emplace_back(std::move(node));
		}
		{
			ZE_MAKE_NODE_DATA("ssao", QueueType::Compute, SSAO, dev, buildData, worldData.DynamicDataBuffer);
			node.AddInput("lambertian.DS", Resource::State::ShaderResourceNonPS);
			node.AddInput("lambertian.GB_N", Resource::State::ShaderResourceNonPS);
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
			ZE_MAKE_NODE_DATA("outlineDraw", QueueType::Main, OutlineDraw, dev, buildData, worldData,
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
			ZE_MAKE_NODE_DATA("wireframe", QueueType::Main, Wireframe, dev, buildData, worldData,
				frameBufferDesc.GetFormat(rawScene), frameBufferDesc.GetFormat(gbuffDepth));
			node.AddInput("verticalBlur.RT", Resource::State::RenderTarget);
			node.AddInput("skybox.DS", Resource::State::DepthWrite);
			node.AddOutput("RT", Resource::State::RenderTarget, rawScene);
			nodes.emplace_back(std::move(node));
		}
#pragma endregion
#pragma region Post processing
		{
			ZE_MAKE_NODE_DATA("skybox", QueueType::Main, Skybox, dev, buildData, worldData.DynamicDataBuffer,
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

		worldData.MeshInfo.Size = 0;
		worldData.MeshInfo.Allocated = MESH_LIST_GROW_SIZE;
		worldData.Meshes = Table::Create<Info::Mesh>(worldData.MeshInfo);

		worldData.ShadowCasterInfo = worldData.MeshInfo;
		worldData.ShadowCasters = Table::Create<Info::Mesh>(worldData.ShadowCasterInfo);

		worldData.OutlineInfo = worldData.MeshInfo;
		worldData.Outlines = Table::Create<Info::Mesh>(worldData.OutlineInfo);

		worldData.WireframeInfo = worldData.MeshInfo;
		worldData.Wireframes = Table::Create<Info::Mesh>(worldData.WireframeInfo);

		worldData.DirectionalLightInfo.Size = 0;
		worldData.DirectionalLightInfo.Allocated = DIR_LIGHT_LIST_GROW_SIZE;
		worldData.DirectionalLights = Table::Create<Info::Light>(worldData.DirectionalLightInfo);

		worldData.SpotLightInfo.Size = 0;
		worldData.SpotLightInfo.Allocated = SPOT_LIGHT_LIST_GROW_SIZE;
		worldData.SpotLights = Table::Create<Info::Light>(worldData.SpotLightInfo);

		worldData.PointLightInfo.Size = 0;
		worldData.PointLightInfo.Allocated = POINT_LIGHT_LIST_GROW_SIZE;
		worldData.PointLights = Table::Create<Info::Light>(worldData.PointLightInfo);

		worldData.DynamicDataBuffer.Init(dev, nullptr, sizeof(Info::DynamicWorldData), true);
		dev.EndUploadRegion();
	}

	void RendererPBR::UpdateWorldData(Device& dev, Data::EID camera) noexcept
	{
		ZE_ASSERT(worldData.ActiveScene, "No active scene set!");
		ZE_ASSERT(worldData.ActiveScene->CameraPositions.contains(camera),
			"Current camera not present!");
		ZE_ASSERT(worldData.ActiveScene->TransformPositions.contains(camera),
			"Current camera does not have Transform component!");

		// Setup shader world data
		Info::DynamicWorldData data;
		data.CameraPos = worldData.ActiveScene->TransformsGlobal[worldData.ActiveScene->TransformPositions.at(camera)].Position;
		const Data::Camera& currentCamera = worldData.ActiveScene->Cameras[worldData.ActiveScene->CameraPositions.at(camera)];
		data.NearClip = currentCamera.Projection.NearClip;
		data.FarClip = currentCamera.Projection.FarClip;
		data.ViewProjection =
			Math::XMMatrixLookToLH(Math::XMLoadFloat3(&data.CameraPos),
				Math::XMLoadFloat3(&currentCamera.EyeDirection),
				Math::XMLoadFloat3(&currentCamera.UpVector)) *
			worldData.ActiveScene->CurrentProjection;
		data.ViewProjectionInverse = Math::XMMatrixTranspose(Math::XMMatrixInverse(nullptr, data.ViewProjection));
		data.ViewProjection = Math::XMMatrixTranspose(data.ViewProjection);
		worldData.DynamicDataBuffer.Update(dev, &data, sizeof(Info::DynamicWorldData));

		// Directional lights
		const Data::LocationLookup<Data::EID>& transformPositions = worldData.ActiveScene->TransformPositions;
		U64 count = worldData.ActiveScene->DirectionalLightInfo.Size;
		const Data::EID* entities = worldData.ActiveScene->DirectionalLightEntities;
		for (U64 i = 0; i < count; ++i)
		{
			ZE_ASSERT(transformPositions.contains(entities[i]), "Entity not containing required Transform component!");

			Table::Append<DIR_LIGHT_LIST_GROW_SIZE>(worldData.DirectionalLightInfo, worldData.DirectionalLights,
				Info::Light(transformPositions.at(entities[i])));
		}

		// Spot lights
		count = worldData.ActiveScene->SpotLightInfo.Size;
		entities = worldData.ActiveScene->SpotLightEntities;
		for (U64 i = 0; i < count; ++i)
		{
			ZE_ASSERT(transformPositions.contains(entities[i]), "Entity not containing required Transform component!");

			Table::Append<SPOT_LIGHT_LIST_GROW_SIZE>(worldData.SpotLightInfo, worldData.SpotLights,
				Info::Light(transformPositions.at(entities[i])));
		}

		// Point lights
		count = worldData.ActiveScene->PointLightInfo.Size;
		entities = worldData.ActiveScene->PointLightEntities;
		for (U64 i = 0; i < count; ++i)
		{
			ZE_ASSERT(transformPositions.contains(entities[i]), "Entity not containing required Transform component!");

			Table::Append<POINT_LIGHT_LIST_GROW_SIZE>(worldData.PointLightInfo, worldData.PointLights,
				Info::Light(transformPositions.at(entities[i])));
		}

		// Clear last batch
		worldData.MeshInfo.Size = 0;
		worldData.ShadowCasterInfo.Size = 0;
		worldData.OutlineInfo.Size = 0;
		worldData.WireframeInfo.Size = 0;
		worldData.DirectionalLightInfo.Size = 0;
		worldData.SpotLightInfo.Size = 0;
		worldData.PointLightInfo.Size = 0;

		// TODO: Add some frustum culling
		// Meshes
		count = worldData.ActiveScene->ModelInfo.Size;
		entities = worldData.ActiveScene->ModelEntities;
		const Data::Model* models = worldData.ActiveScene->Models;
		const Data::Mesh* meshes = worldData.ActiveScene->Meshes;
		for (U64 i = 0, j = 0; i < count; ++i)
		{
			ZE_ASSERT(transformPositions.contains(entities[i]), "Entity not containing required Transform component!");

			Data::Model model = models[i];
			U64 transformIndex = transformPositions.at(entities[i]);

			for (U64 k = 0; k < model.MeshCount; ++j, ++k)
			{
				Data::Mesh mesh = meshes[model.MeshIDs[k]];

				Info::Mesh info
				{
					mesh.GeometryIndex,
					mesh.MaterialIndex,
					transformIndex
				};

				if (mesh.Flags & Data::MeshFlag::Shadow)
					Table::Append<MESH_LIST_GROW_SIZE>(worldData.ShadowCasterInfo, worldData.ShadowCasters, info);

				if (mesh.Flags & Data::MeshFlag::Outline)
					Table::Append<MESH_LIST_GROW_SIZE>(worldData.OutlineInfo, worldData.Outlines, info);

				if (mesh.Flags & Data::MeshFlag::Wireframe)
					Table::Append<MESH_LIST_GROW_SIZE>(worldData.WireframeInfo, worldData.Wireframes, info);
				else
					Table::Append<MESH_LIST_GROW_SIZE>(worldData.MeshInfo, worldData.Meshes, info);
			}
		}
	}
}