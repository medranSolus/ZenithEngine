#include "GFX/Pipeline/RendererPBR.h"
#include "GFX/Pipeline/RenderPasses.h"

#define ZE_MAKE_NODE(name, queueType, passNamespace) RenderNode node(name, queueType, RenderPass::##passNamespace##::Execute)
#define ZE_MAKE_NODE_STATIC(name, queueType, passNamespace, ...) RenderNode node(name, queueType, nullptr, nullptr, RenderPass::##passNamespace##::Execute(__VA_ARGS__), true)
#define ZE_MAKE_NODE_DATA(name, queueType, passNamespace, ...) RenderNode node(name, queueType, RenderPass::passNamespace::Execute, RenderPass::passNamespace::Clean, RenderPass::passNamespace::Setup(__VA_ARGS__))

namespace ZE::GFX::Pipeline
{
	RendererPBR::~RendererPBR()
	{
		if (worldData.Meshes)
			Table::Clear(worldData.MeshesInfo.Size, worldData.Meshes);
	}

	void RendererPBR::Init(Device& dev, CommandList& mainList, Resource::Texture::Library& texLib,
		U32 width, U32 height, bool minimizePassDistances, U32 shadowMapSize)
	{
		const U32 outlineBuffWidth = width / 2;
		const U32 outlineBuffHeight = height / 2;
		FrameBufferDesc frameBufferDesc;
		frameBufferDesc.Init(11, width, height);

#pragma region Framebuffer definition
		const RID gbuffColor = frameBufferDesc.AddResource(
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::R8G8B8A8_UNorm, ColorF4() });
		const RID gbuffNormal = frameBufferDesc.AddResource(
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::R32G32_Float, ColorF4() });
		const RID gbuffSpecular = frameBufferDesc.AddResource(
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::R16G16B16A16_Float, ColorF4() });
		const RID gbuffDepth = frameBufferDesc.AddResource(
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::DepthOnly, ColorF4(), 0.0f, 0 });
		const RID lightbuffColor = frameBufferDesc.AddResource(
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::R16G16B16A16_Float, ColorF4() });
		const RID lightbuffSpecular = frameBufferDesc.AddResource(
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::R16G16B16A16_Float, ColorF4() });
		const RID rawScene = frameBufferDesc.AddResource(
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::R16G16B16A16_Float, ColorF4() });
		const RID ssao = frameBufferDesc.AddResource(
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::R32_Float, ColorF4() });
		const RID outline = frameBufferDesc.AddResource(
			{ outlineBuffWidth, outlineBuffHeight, 1, FrameResourceFlags::None, Settings::GetBackbufferFormat(), ColorF4() });
		const RID outlineBlur = frameBufferDesc.AddResource(
			{ outlineBuffWidth, outlineBuffHeight, 1, FrameResourceFlags::None, Settings::GetBackbufferFormat(), ColorF4() });
		const RID outlineDepth = frameBufferDesc.AddResource(
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::DepthStencil, ColorF4(), 0.0f, 0 });
#pragma endregion

		settingsData.FarClip = 1000.0f;
		settingsData.NearClip = 0.001f;
		settingsData.HDRExposure = 1.0f;
		settingsData.Gamma = 2.2f;
		settingsData.GammaInverse = 1.0f / 2.2f;
		settingsData.ViewProjection = Math::XMMatrixIdentity();
		settingsData.ViewProjectionInverse = Math::XMMatrixIdentity();
		dev.StartUpload();
		settingsBuffer.Init(dev, &settingsData, sizeof(PBRData), false);
		dev.FinishUpload();

		std::vector<GFX::Pipeline::RenderNode> nodes;
		RendererBuildData buildData = { bindings, texLib };
#pragma region Renderer bindings
		buildData.RendererSlots.AddRange({ 1, 13, Resource::ShaderType::Pixel, Binding::RangeFlag::CBV });

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
		buildData.RendererSlots.AddSampler(
			{
				Resource::SamplerType::Anisotropic,
				{
					Resource::Texture::AddressMode::Repeat,
					Resource::Texture::AddressMode::Repeat,
					Resource::Texture::AddressMode::Repeat
				},
				0.0f, 4, Resource::CompareMethod::Never,
				Resource::Texture::EdgeColor::TransparentBlack,
				0.0f, FLT_MAX, 1
			});
		buildData.RendererSlots.AddSampler(
			{
				Resource::SamplerType::Anisotropic,
				{
					Resource::Texture::AddressMode::Mirror,
					Resource::Texture::AddressMode::Mirror,
					Resource::Texture::AddressMode::Mirror
				},
				0.0f, 4, Resource::CompareMethod::Never,
				Resource::Texture::EdgeColor::TransparentBlack,
				0.0f, FLT_MAX, 2
			});

		buildData.RendererSlots.AddSampler(
			{
				Resource::SamplerType::Linear,
				{
					Resource::Texture::AddressMode::Repeat,
					Resource::Texture::AddressMode::Repeat,
					Resource::Texture::AddressMode::Repeat
				},
				0.0f, 4, Resource::CompareMethod::Never,
				Resource::Texture::EdgeColor::TransparentBlack,
				0.0f, FLT_MAX, 3
			});
		buildData.RendererSlots.AddSampler(
			{
				Resource::SamplerType::Linear,
				{
					Resource::Texture::AddressMode::Mirror,
					Resource::Texture::AddressMode::Mirror,
					Resource::Texture::AddressMode::Mirror
				},
				0.0f, 4, Resource::CompareMethod::Never,
				Resource::Texture::EdgeColor::TransparentBlack,
				0.0f, FLT_MAX, 4
			});

		buildData.RendererSlots.AddSampler(
			{
				Resource::SamplerType::Point,
				{
					Resource::Texture::AddressMode::Repeat,
					Resource::Texture::AddressMode::Repeat,
					Resource::Texture::AddressMode::Repeat
				},
				0.0f, 4, Resource::CompareMethod::Never,
				Resource::Texture::EdgeColor::TransparentBlack,
				0.0f, FLT_MAX, 5
			});
		buildData.RendererSlots.AddSampler(
			{
				Resource::SamplerType::Point,
				{
					Resource::Texture::AddressMode::Mirror,
					Resource::Texture::AddressMode::Mirror,
					Resource::Texture::AddressMode::Mirror
				},
				0.0f, 4, Resource::CompareMethod::Never,
				Resource::Texture::EdgeColor::TransparentBlack,
				0.0f, FLT_MAX, 6
			});
#pragma endregion

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
			GFX::Pipeline::RenderNode node("dirLight", GFX::QueueType::Main, nullptr);
			node.AddInput("lambertian.DS", Resource::State::ShaderResourcePS);
			node.AddInput("lambertian.GB_C", Resource::State::ShaderResourcePS);
			node.AddInput("lambertian.GB_N", Resource::State::ShaderResourcePS);
			node.AddInput("lambertian.GB_S", Resource::State::ShaderResourcePS);
			//node.AddInnerBuffer(Resource::State::RenderTarget,
			//	{ shadowMapSize, shadowMapSize, 1, FrameResourceFlags::None, PixelFormat::R32_Float, ColorF4() });
			//node.AddInnerBuffer(Resource::State::DepthWrite,
			//	{ shadowMapSize, shadowMapSize, 1, FrameResourceFlags::None, PixelFormat::DepthOnly, ColorF4(), 0.0f, 0 });
			node.AddOutput("LB_C", Resource::State::RenderTarget, lightbuffColor);
			node.AddOutput("LB_S", Resource::State::RenderTarget, lightbuffSpecular);
			nodes.emplace_back(std::move(node));
		}
		{
			GFX::Pipeline::RenderNode node("spotLight", GFX::QueueType::Main, nullptr);
			node.AddInput("lambertian.DS", Resource::State::ShaderResourcePS);
			node.AddInput("lambertian.GB_C", Resource::State::ShaderResourcePS);
			node.AddInput("lambertian.GB_N", Resource::State::ShaderResourcePS);
			node.AddInput("lambertian.GB_S", Resource::State::ShaderResourcePS);
			node.AddInput("dirLight.LB_C", Resource::State::RenderTarget);
			node.AddInput("dirLight.LB_S", Resource::State::RenderTarget);
			node.AddInnerBuffer(Resource::State::RenderTarget,
				{ shadowMapSize, shadowMapSize, 1, FrameResourceFlags::None, PixelFormat::R32_Float, ColorF4() });
			node.AddInnerBuffer(Resource::State::DepthWrite,
				{ shadowMapSize, shadowMapSize, 1, FrameResourceFlags::None, PixelFormat::DepthOnly, ColorF4(), 0.0f, 0 });
			node.AddOutput("LB_C", Resource::State::RenderTarget, lightbuffColor);
			node.AddOutput("LB_S", Resource::State::RenderTarget, lightbuffSpecular);
			nodes.emplace_back(std::move(node));
		}
		{
			GFX::Pipeline::RenderNode node("pointLight", GFX::QueueType::Main, nullptr);
			node.AddInput("lambertian.DS", Resource::State::ShaderResourcePS);
			node.AddInput("lambertian.GB_C", Resource::State::ShaderResourcePS);
			node.AddInput("lambertian.GB_N", Resource::State::ShaderResourcePS);
			node.AddInput("lambertian.GB_S", Resource::State::ShaderResourcePS);
			node.AddInput("spotLight.LB_C", Resource::State::RenderTarget);
			node.AddInput("spotLight.LB_S", Resource::State::RenderTarget);
			node.AddInnerBuffer(Resource::State::RenderTarget,
				{ shadowMapSize, shadowMapSize, 1, FrameResourceFlags::Cube, PixelFormat::R32_Float, ColorF4() });
			node.AddInnerBuffer(Resource::State::DepthWrite,
				{ shadowMapSize, shadowMapSize, 1, FrameResourceFlags::Cube, PixelFormat::DepthOnly, ColorF4(), 0.0f, 0 });
			node.AddOutput("LB_C", Resource::State::RenderTarget, lightbuffColor);
			node.AddOutput("LB_S", Resource::State::RenderTarget, lightbuffSpecular);
			nodes.emplace_back(std::move(node));
		}
		{
			GFX::Pipeline::RenderNode node("ssao", GFX::QueueType::Compute, nullptr);
			node.AddInput("lambertian.DS", Resource::State::ShaderResourceNonPS);
			node.AddInput("lambertian.GB_N", Resource::State::ShaderResourceNonPS);
			node.AddOutput("SB", Resource::State::UnorderedAccess, ssao);
			nodes.emplace_back(std::move(node));
		}
		{
			ZE_MAKE_NODE_DATA("lightCombine", QueueType::Main, LightCombine, dev, buildData, frameBufferDesc.GetFormat(rawScene));
			node.AddInput("ssao.SB", Resource::State::ShaderResourcePS);
			node.AddInput("pointLight.LB_C", Resource::State::ShaderResourcePS);
			node.AddInput("pointLight.LB_S", Resource::State::ShaderResourcePS);
			node.AddInput("lambertian.GB_C", Resource::State::ShaderResourcePS);
			node.AddOutput("RT", Resource::State::RenderTarget, rawScene);
			nodes.emplace_back(std::move(node));
		}
#pragma endregion
#pragma region Geometry effects
		{
			ZE_MAKE_NODE_DATA("outlineDraw", QueueType::Main, OutlineDraw, dev, buildData, frameBufferDesc.GetFormat(outlineBlur), frameBufferDesc.GetFormat(outlineDepth));
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
			ZE_MAKE_NODE_DATA("wireframe", QueueType::Main, Wireframe, dev, buildData, frameBufferDesc.GetFormat(rawScene), frameBufferDesc.GetFormat(gbuffDepth));
			node.AddInput("verticalBlur.RT", Resource::State::RenderTarget);
			node.AddInput("skybox.DS", Resource::State::DepthWrite);
			node.AddOutput("RT", Resource::State::RenderTarget, rawScene);
			nodes.emplace_back(std::move(node));
		}
#pragma endregion
#pragma region Post processing
		{
			ZE_MAKE_NODE_DATA("skybox", QueueType::Main, Skybox, dev, buildData, frameBufferDesc.GetFormat(rawScene), frameBufferDesc.GetFormat(gbuffDepth));
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
		Finalize(dev, mainList, nodes, frameBufferDesc, buildData, minimizePassDistances);

		worldData.MeshesInfo.Size = 0;
		worldData.MeshesInfo.Allocated = MESH_LIST_GROW_SIZE;
		worldData.Meshes = Table::Create<MeshInfo>(MESH_LIST_GROW_SIZE);
	}

	void RendererPBR::SetCurrentCamera(Device& dev, Data::EID camera)
	{
		ZE_ASSERT(worldData.ActiveScene->CameraPositions.contains(camera), "Camera not present in the scene!");

		worldData.CurrnetCamera = camera;

		auto& cameraData = worldData.ActiveScene->Cameras[worldData.ActiveScene->CameraPositions.at(camera)];
		// TODO: Set camera data
		dev.StartUpload();
		settingsBuffer.Update(dev, &settingsData, sizeof(PBRData));
		dev.FinishUpload();
	}

	void RendererPBR::UpdateWorldData() noexcept
	{
		ZE_ASSERT(worldData.ActiveScene, "No active scene set!");
		ZE_ASSERT(worldData.ActiveScene->CameraPositions.contains(worldData.CurrnetCamera),
			"Current camera not present!");

		U64 modelCount = worldData.ActiveScene->ModelInfo.Size;
		const Data::EID* modelEntities = worldData.ActiveScene->ModelEntities;
		const Data::Model* models = worldData.ActiveScene->Models;
		const Data::LocationLookup<Data::EID>& transformPositions = worldData.ActiveScene->TransformPositions;
		const Data::Mesh* meshes = worldData.ActiveScene->Meshes;

		// TODO: Add some culling
		U64 meshCount = 0;
		for (U64 i = 0; i < modelCount; ++i)
		{
			ZE_ASSERT(transformPositions.contains(modelEntities[i]), "Entity not containing required Transform component!");
			meshCount += models[i].MeshCount;
		}

		if (meshCount > worldData.MeshesInfo.Size + MESH_LIST_GROW_SIZE || worldData.MeshesInfo.Size > meshCount + MESH_LIST_GROW_SIZE)
			Table::Resize(worldData.MeshesInfo, worldData.Meshes, meshCount);
		else
			worldData.MeshesInfo.Size = meshCount;

		for (U64 i = 0, j = 0; i < modelCount; ++i)
		{
			Data::Model model = models[i];
			U64 transformIndex = transformPositions.at(modelEntities[i]);

			for (U64 k = 0; k < model.MeshCount; ++j, ++k)
			{
				MeshInfo& info = worldData.Meshes[j];
				Data::Mesh mesh = meshes[model.MeshIDs[k]];

				info.GeometryIndex = mesh.GeometryIndex;
				info.MaterialIndex = mesh.MaterialIndex;
				info.TransformIndex = transformIndex;
			}
		}
	}
}