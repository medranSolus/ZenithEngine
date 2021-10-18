#include "GFX/Pipeline/RendererPBR.h"

namespace ZE::GFX::Pipeline
{
	void RendererPBR::Init(U32 width, U32 height, U32 shadowMapSize)
	{
		constexpr const char* GBUFF_NAME = "geometryBuffer";
		constexpr const char* GBUFF_DEPTH_NAME = "geometryBufferDepth";
		constexpr const char* LBUFF_NAME = "lightBuffer";
		constexpr const char* SCENE_BUFF_NAME = "rawScene";
		constexpr const char* SSAO_BUFF_NAME = "ssaoBuffer";
		constexpr const char* OUTLINE_BUFF_NAME = "outlineBuffer";
		constexpr const char* OUTLINE_BLUR_BUFF_NAME = "outlineBlurBuffer";
		constexpr const char* OUTLINE_BUFF_DEPTH_NAME = "outlineDepthBuffer";

		const U32 outlineBuffWidth = width / 2;
		const U32 outlineBuffHeight = height / 2;
		Resource::FrameBufferDesc frameBufferDesc;
		frameBufferDesc.Reserve(8);
		frameBufferDesc.AddResource(GBUFF_NAME,
			{
				width, height, 1,
				{ PixelFormat::R8G8B8A8_UNorm, PixelFormat::R32G32_Float, PixelFormat::R16G16B16A16_Float }
			});
		frameBufferDesc.AddResource(GBUFF_DEPTH_NAME,
			{ width, height, 1, { PixelFormat::DepthOnly } });
		frameBufferDesc.AddResource(LBUFF_NAME,
			{ width, height, 1, { PixelFormat::R16G16B16A16_Float, PixelFormat::R16G16B16A16_Float } });
		frameBufferDesc.AddResource(SCENE_BUFF_NAME,
			{ width, height, 1, { PixelFormat::R16G16B16A16_Float } });
		frameBufferDesc.AddResource(SSAO_BUFF_NAME,
			{ width, height, 1, { PixelFormat::R32_Float } });
		frameBufferDesc.AddResource(OUTLINE_BUFF_NAME,
			{ outlineBuffWidth, outlineBuffHeight, 1, { Settings::GetBackbufferFormat() } });
		frameBufferDesc.AddResource(OUTLINE_BLUR_BUFF_NAME,
			{ outlineBuffWidth, outlineBuffHeight, 1, { Settings::GetBackbufferFormat() } });
		frameBufferDesc.AddResource(OUTLINE_BUFF_DEPTH_NAME,
			{ width, height, 1, { PixelFormat::DepthStencil } });

		std::vector<GFX::Pipeline::RenderNode> nodes;
#pragma region Geometry
		{
			GFX::Pipeline::RenderNode node("lambertianDepth", GFX::QueueType::Main, nullptr);
			node.AddOutput("DS", Resource::State::DepthWrite, GBUFF_DEPTH_NAME);
			node.AddOutput("GB", Resource::State::RenderTarget, GBUFF_NAME);
			nodes.emplace_back(std::move(node));
		}
		{
			GFX::Pipeline::RenderNode node("lambertianClassic", GFX::QueueType::Main, nullptr);
			node.AddInput("lambertianDepth.DS", Resource::State::DepthWrite);
			node.AddInput("lambertianDepth.GB", Resource::State::RenderTarget);
			node.AddOutput("DS", Resource::State::DepthWrite, GBUFF_DEPTH_NAME);
			node.AddOutput("GB", Resource::State::RenderTarget, GBUFF_NAME);
			nodes.emplace_back(std::move(node));
		}
#pragma endregion
#pragma region Lightning
		{
			GFX::Pipeline::RenderNode node("dirLight", GFX::QueueType::Main, nullptr);
			node.AddInput("lambertianClassic.DS", Resource::State::ShaderResourcePS);
			node.AddInput("lambertianClassic.GB", Resource::State::ShaderResourcePS);
			//node.AddInnerBuffer("shadowMap", Resource::State::RenderTarget, { shadowMapSize, shadowMapSize, 1, { PixelFormat::R32_Float } });
			//node.AddInnerBuffer("shadowMapDepth", { shadowMapSize, shadowMapSize, 1, { PixelFormat::DepthOnly } });
			node.AddOutput("LB", Resource::State::RenderTarget, LBUFF_NAME);
			nodes.emplace_back(std::move(node));
		}
		{
			GFX::Pipeline::RenderNode node("spotLight", GFX::QueueType::Main, nullptr);
			node.AddInput("lambertianClassic.DS", Resource::State::ShaderResourcePS);
			node.AddInput("lambertianClassic.GB", Resource::State::ShaderResourcePS);
			node.AddInput("dirLight.LB", Resource::State::RenderTarget);
			node.AddInnerBuffer("shadowMap", Resource::State::RenderTarget, { shadowMapSize, shadowMapSize, 1, { PixelFormat::R32_Float } });
			node.AddInnerBuffer("shadowMapDepth", Resource::State::DepthWrite, { shadowMapSize, shadowMapSize, 1, { PixelFormat::DepthOnly } });
			node.AddOutput("LB", Resource::State::RenderTarget, LBUFF_NAME);
			nodes.emplace_back(std::move(node));
		}
		{
			GFX::Pipeline::RenderNode node("pointLight", GFX::QueueType::Main, nullptr);
			node.AddInput("lambertianClassic.DS", Resource::State::ShaderResourcePS);
			node.AddInput("lambertianClassic.GB", Resource::State::ShaderResourcePS);
			node.AddInput("spotLight.LB", Resource::State::RenderTarget);
			node.AddInnerBuffer("shadowMap", Resource::State::RenderTarget, { shadowMapSize, shadowMapSize, 6, { PixelFormat::R32_Float } });
			node.AddInnerBuffer("shadowMapDepth", Resource::State::DepthWrite, { shadowMapSize, shadowMapSize, 6, { PixelFormat::DepthOnly } });
			node.AddOutput("LB", Resource::State::RenderTarget, LBUFF_NAME);
			nodes.emplace_back(std::move(node));
		}
		{
			GFX::Pipeline::RenderNode node("ssao", GFX::QueueType::Compute, nullptr);
			node.AddInput("lambertianClassic.DS", Resource::State::ShaderResourceNonPS);
			node.AddInput("lambertianClassic.GB", Resource::State::ShaderResourceNonPS);
			node.AddOutput("SB", Resource::State::UnorderedAccess, SSAO_BUFF_NAME);
			nodes.emplace_back(std::move(node));
		}
		{
			GFX::Pipeline::RenderNode node("lightCombine", GFX::QueueType::Main, nullptr);
			node.AddInput("ssao.SB", Resource::State::ShaderResourcePS);
			node.AddInput("pointLight.LB", Resource::State::ShaderResourcePS);
			node.AddInput("lambertianClassic.GB", Resource::State::ShaderResourcePS);
			node.AddOutput("RT", Resource::State::RenderTarget, SCENE_BUFF_NAME);
			nodes.emplace_back(std::move(node));
		}
#pragma endregion
#pragma region Geometry effects
		{
			GFX::Pipeline::RenderNode node("outlineDraw", GFX::QueueType::Main, nullptr);
			node.AddOutput("RT", Resource::State::RenderTarget, OUTLINE_BUFF_NAME);
			node.AddOutput("DS", Resource::State::DepthWrite, OUTLINE_BUFF_DEPTH_NAME);
			nodes.emplace_back(std::move(node));
		}
		{
			GFX::Pipeline::RenderNode node("horizontalBlur", GFX::QueueType::Main, nullptr);
			node.AddInput("outlineDraw.RT", Resource::State::ShaderResourcePS);
			node.AddOutput("RT", Resource::State::RenderTarget, OUTLINE_BLUR_BUFF_NAME);
			nodes.emplace_back(std::move(node));
		}
		{
			GFX::Pipeline::RenderNode node("verticalBlur", GFX::QueueType::Main, nullptr);
			node.AddInput("horizontalBlur.RT", Resource::State::ShaderResourcePS);
			node.AddInput("outlineDraw.DS", Resource::State::DepthRead);
			node.AddInput("skybox.RT", Resource::State::RenderTarget);
			node.AddOutput("RT", Resource::State::RenderTarget, SCENE_BUFF_NAME);
			nodes.emplace_back(std::move(node));
		}
		{
			GFX::Pipeline::RenderNode node("wireframe", GFX::QueueType::Main, nullptr);
			node.AddInput("verticalBlur.RT", Resource::State::RenderTarget);
			node.AddInput("skybox.DS", Resource::State::DepthWrite);
			node.AddOutput("RT", Resource::State::RenderTarget, SCENE_BUFF_NAME);
			nodes.emplace_back(std::move(node));
		}
#pragma endregion
#pragma region Post processing
		{
			GFX::Pipeline::RenderNode node("skybox", GFX::QueueType::Main, nullptr);
			node.AddInput("lightCombine.RT", Resource::State::RenderTarget);
			node.AddInput("lambertianClassic.DS", Resource::State::DepthRead);
			node.AddOutput("RT", Resource::State::RenderTarget, SCENE_BUFF_NAME);
			node.AddOutput("DS", Resource::State::DepthRead, GBUFF_DEPTH_NAME);
			nodes.emplace_back(std::move(node));
		}
		{
			GFX::Pipeline::RenderNode node("hdrGamma", GFX::QueueType::Main, nullptr);
			node.AddInput("wireframe.RT", Resource::State::ShaderResourcePS);
			node.AddOutput("RT", Resource::State::Present, BACKBUFFER_NAME);
			nodes.emplace_back(std::move(node));
		}
#pragma endregion
		Finalize(nodes, frameBufferDesc);
	}
}