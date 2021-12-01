#include "GFX/Pipeline/RendererPBR.h"

namespace ZE::GFX::Pipeline
{
	void RendererPBR::Init(Device& dev, CommandList& mainList, U32 width, U32 height, bool minimizePassDistances, U32 shadowMapSize)
	{
		constexpr const char* BUFFNAME_GBUFF_COLOR = "gbuffCol";
		constexpr const char* BUFFNAME_GBUFF_NORMAL = "gbuffNormal";
		constexpr const char* BUFFNAME_GBUFF_SPECULAR = "gbuffSpec";
		constexpr const char* BUFFNAME_GBUFF_DEPTH = "gbuffZ";
		constexpr const char* BUFFNAME_LIGHT_COLOR = "lbuffCol";
		constexpr const char* BUFFNAME_LIGHT_SPECULAR = "lbuffSpec";
		constexpr const char* BUFFNAME_SCENE = "scene";
		constexpr const char* BUFFNAME_SSAO = "ssao";
		constexpr const char* BUFFNAME_OUTLINE = "outl";
		constexpr const char* BUFFNAME_OUTLINE_BLUR = "outlBlur";
		constexpr const char* BUFFNAME_OUTLINE_DEPTH = "outlZ";

		const U32 outlineBuffWidth = width / 2;
		const U32 outlineBuffHeight = height / 2;
		FrameBufferDesc frameBufferDesc;
		frameBufferDesc.Init(8, BACKBUFFER_NAME, width, height);

#pragma region Framebuffer
		frameBufferDesc.AddResource(BUFFNAME_GBUFF_COLOR,
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::R8G8B8A8_UNorm, ColorF4() });
		frameBufferDesc.AddResource(BUFFNAME_GBUFF_NORMAL,
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::R32G32_Float, ColorF4() });
		frameBufferDesc.AddResource(BUFFNAME_GBUFF_SPECULAR,
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::R16G16B16A16_Float, ColorF4() });
		frameBufferDesc.AddResource(BUFFNAME_GBUFF_DEPTH,
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::DepthOnly, ColorF4(), 0.0f, 0 });
		frameBufferDesc.AddResource(BUFFNAME_LIGHT_COLOR,
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::R16G16B16A16_Float, ColorF4() });
		frameBufferDesc.AddResource(BUFFNAME_LIGHT_SPECULAR,
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::R16G16B16A16_Float, ColorF4() });
		frameBufferDesc.AddResource(BUFFNAME_SCENE,
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::R16G16B16A16_Float, ColorF4() });
		frameBufferDesc.AddResource(BUFFNAME_SSAO,
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::R32_Float, ColorF4() });
		frameBufferDesc.AddResource(BUFFNAME_OUTLINE,
			{ outlineBuffWidth, outlineBuffHeight, 1, FrameResourceFlags::None, Settings::GetBackbufferFormat(), ColorF4() });
		frameBufferDesc.AddResource(BUFFNAME_OUTLINE_BLUR,
			{ outlineBuffWidth, outlineBuffHeight, 1, FrameResourceFlags::None, Settings::GetBackbufferFormat(), ColorF4() });
		frameBufferDesc.AddResource(BUFFNAME_OUTLINE_DEPTH,
			{ width, height, 1, FrameResourceFlags::None, PixelFormat::DepthStencil, ColorF4(), 0.0f, 0 });
#pragma endregion

		std::vector<GFX::Pipeline::RenderNode> nodes;
#pragma region Geometry
		{
			GFX::Pipeline::RenderNode node("lambertianDepth", GFX::QueueType::Main, nullptr);
			node.AddOutput("DS", Resource::State::DepthWrite, BUFFNAME_GBUFF_DEPTH);
			node.AddOutput("GB_C", Resource::State::RenderTarget, BUFFNAME_GBUFF_COLOR);
			node.AddOutput("GB_N", Resource::State::RenderTarget, BUFFNAME_GBUFF_NORMAL);
			node.AddOutput("GB_S", Resource::State::RenderTarget, BUFFNAME_GBUFF_SPECULAR);
			nodes.emplace_back(std::move(node));
		}
		{
			GFX::Pipeline::RenderNode node("lambertianClassic", GFX::QueueType::Main, nullptr);
			node.AddInput("lambertianDepth.DS", Resource::State::DepthWrite);
			node.AddInput("lambertianDepth.GB_C", Resource::State::RenderTarget);
			node.AddInput("lambertianDepth.GB_N", Resource::State::RenderTarget);
			node.AddInput("lambertianDepth.GB_S", Resource::State::RenderTarget);
			node.AddOutput("DS", Resource::State::DepthWrite, BUFFNAME_GBUFF_DEPTH);
			node.AddOutput("GB_C", Resource::State::RenderTarget, BUFFNAME_GBUFF_COLOR);
			node.AddOutput("GB_N", Resource::State::RenderTarget, BUFFNAME_GBUFF_NORMAL);
			node.AddOutput("GB_S", Resource::State::RenderTarget, BUFFNAME_GBUFF_SPECULAR);
			nodes.emplace_back(std::move(node));
		}
#pragma endregion
#pragma region Lightning
		{
			GFX::Pipeline::RenderNode node("dirLight", GFX::QueueType::Main, nullptr);
			node.AddInput("lambertianClassic.DS", Resource::State::ShaderResourcePS);
			node.AddInput("lambertianClassic.GB_C", Resource::State::ShaderResourcePS);
			node.AddInput("lambertianClassic.GB_N", Resource::State::ShaderResourcePS);
			node.AddInput("lambertianClassic.GB_S", Resource::State::ShaderResourcePS);
			//node.AddInnerBuffer("shadowMap", Resource::State::RenderTarget, { shadowMapSize, shadowMapSize, 1, FrameResourceFlags::None, { PixelFormat::R32_Float } });
			//node.AddInnerBuffer("shadowMapDepth", Resource::State::DepthWrite, { shadowMapSize, shadowMapSize, 1, FrameResourceFlags::None, { PixelFormat::DepthOnly } });
			node.AddOutput("LB_C", Resource::State::RenderTarget, BUFFNAME_LIGHT_COLOR);
			node.AddOutput("LB_S", Resource::State::RenderTarget, BUFFNAME_LIGHT_SPECULAR);
			nodes.emplace_back(std::move(node));
		}
		{
			GFX::Pipeline::RenderNode node("spotLight", GFX::QueueType::Main, nullptr);
			node.AddInput("lambertianClassic.DS", Resource::State::ShaderResourcePS);
			node.AddInput("lambertianClassic.GB_C", Resource::State::ShaderResourcePS);
			node.AddInput("lambertianClassic.GB_N", Resource::State::ShaderResourcePS);
			node.AddInput("lambertianClassic.GB_S", Resource::State::ShaderResourcePS);
			node.AddInput("dirLight.LB_C", Resource::State::RenderTarget);
			node.AddInput("dirLight.LB_S", Resource::State::RenderTarget);
			node.AddInnerBuffer("shadowMap", Resource::State::RenderTarget,
				{ shadowMapSize, shadowMapSize, 1, FrameResourceFlags::None, PixelFormat::R32_Float, ColorF4() });
			node.AddInnerBuffer("shadowMapDepth", Resource::State::DepthWrite,
				{ shadowMapSize, shadowMapSize, 1, FrameResourceFlags::None, PixelFormat::DepthOnly, ColorF4(), 0.0f, 0 });
			node.AddOutput("LB_C", Resource::State::RenderTarget, BUFFNAME_LIGHT_COLOR);
			node.AddOutput("LB_S", Resource::State::RenderTarget, BUFFNAME_LIGHT_SPECULAR);
			nodes.emplace_back(std::move(node));
		}
		{
			GFX::Pipeline::RenderNode node("pointLight", GFX::QueueType::Main, nullptr);
			node.AddInput("lambertianClassic.DS", Resource::State::ShaderResourcePS);
			node.AddInput("lambertianClassic.GB_C", Resource::State::ShaderResourcePS);
			node.AddInput("lambertianClassic.GB_N", Resource::State::ShaderResourcePS);
			node.AddInput("lambertianClassic.GB_S", Resource::State::ShaderResourcePS);
			node.AddInput("spotLight.LB_C", Resource::State::RenderTarget);
			node.AddInput("spotLight.LB_S", Resource::State::RenderTarget);
			node.AddInnerBuffer("shadowMap", Resource::State::RenderTarget,
				{ shadowMapSize, shadowMapSize, 1, FrameResourceFlags::Cube, PixelFormat::R32_Float, ColorF4() });
			node.AddInnerBuffer("shadowMapDepth", Resource::State::DepthWrite,
				{ shadowMapSize, shadowMapSize, 1, FrameResourceFlags::Cube, PixelFormat::DepthOnly, ColorF4(), 0.0f, 0 });
			node.AddOutput("LB_C", Resource::State::RenderTarget, BUFFNAME_LIGHT_COLOR);
			node.AddOutput("LB_S", Resource::State::RenderTarget, BUFFNAME_LIGHT_SPECULAR);
			nodes.emplace_back(std::move(node));
		}
		{
			GFX::Pipeline::RenderNode node("ssao", GFX::QueueType::Compute, nullptr);
			node.AddInput("lambertianClassic.DS", Resource::State::ShaderResourceNonPS);
			node.AddInput("lambertianClassic.GB_N", Resource::State::ShaderResourceNonPS);
			node.AddOutput("SB", Resource::State::UnorderedAccess, BUFFNAME_SSAO);
			nodes.emplace_back(std::move(node));
		}
		{
			GFX::Pipeline::RenderNode node("lightCombine", GFX::QueueType::Main, nullptr);
			node.AddInput("ssao.SB", Resource::State::ShaderResourcePS);
			node.AddInput("pointLight.LB_C", Resource::State::ShaderResourcePS);
			node.AddInput("pointLight.LB_S", Resource::State::ShaderResourcePS);
			node.AddInput("lambertianClassic.GB_C", Resource::State::ShaderResourcePS);
			node.AddOutput("RT", Resource::State::RenderTarget, BUFFNAME_SCENE);
			nodes.emplace_back(std::move(node));
		}
#pragma endregion
#pragma region Geometry effects
		{
			GFX::Pipeline::RenderNode node("outlineDraw", GFX::QueueType::Main, nullptr);
			node.AddOutput("RT", Resource::State::RenderTarget, BUFFNAME_OUTLINE);
			node.AddOutput("DS", Resource::State::DepthWrite, BUFFNAME_OUTLINE_DEPTH);
			nodes.emplace_back(std::move(node));
		}
		{
			GFX::Pipeline::RenderNode node("horizontalBlur", GFX::QueueType::Main, nullptr);
			node.AddInput("outlineDraw.RT", Resource::State::ShaderResourcePS);
			node.AddOutput("RT", Resource::State::RenderTarget, BUFFNAME_OUTLINE_BLUR);
			nodes.emplace_back(std::move(node));
		}
		{
			GFX::Pipeline::RenderNode node("verticalBlur", GFX::QueueType::Main, nullptr);
			node.AddInput("horizontalBlur.RT", Resource::State::ShaderResourcePS);
			node.AddInput("outlineDraw.DS", Resource::State::DepthRead);
			node.AddInput("skybox.RT", Resource::State::RenderTarget);
			node.AddOutput("RT", Resource::State::RenderTarget, BUFFNAME_SCENE);
			nodes.emplace_back(std::move(node));
		}
		{
			GFX::Pipeline::RenderNode node("wireframe", GFX::QueueType::Main, nullptr);
			node.AddInput("verticalBlur.RT", Resource::State::RenderTarget);
			node.AddInput("skybox.DS", Resource::State::DepthWrite);
			node.AddOutput("RT", Resource::State::RenderTarget, BUFFNAME_SCENE);
			nodes.emplace_back(std::move(node));
		}
#pragma endregion
#pragma region Post processing
		{
			GFX::Pipeline::RenderNode node("skybox", GFX::QueueType::Main, nullptr);
			node.AddInput("lightCombine.RT", Resource::State::RenderTarget);
			node.AddInput("lambertianClassic.DS", Resource::State::DepthRead);
			node.AddOutput("RT", Resource::State::RenderTarget, BUFFNAME_SCENE);
			node.AddOutput("DS", Resource::State::DepthRead, BUFFNAME_GBUFF_DEPTH);
			nodes.emplace_back(std::move(node));
		}
		{
			GFX::Pipeline::RenderNode node("hdrGamma", GFX::QueueType::Main, nullptr);
			node.AddInput("wireframe.RT", Resource::State::ShaderResourcePS);
			node.AddOutput("RT", Resource::State::RenderTarget, BACKBUFFER_NAME);
			nodes.emplace_back(std::move(node));
		}
#pragma endregion
		Finalize(dev, mainList, nodes, frameBufferDesc, minimizePassDistances);
	}
}