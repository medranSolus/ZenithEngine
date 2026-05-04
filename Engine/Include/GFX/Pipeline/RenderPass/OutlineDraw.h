#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Resource/PipelineStateGfx.h"
#

namespace ZE::GFX::Pipeline::RenderPass::OutlineDraw
{
	// Indicates that entity is inside view frustum
	struct InsideFrustum { Resource::DynamicBufferAlloc Transform; };

	struct Resources
	{
		RID RenderTarget;
		RID DepthStencil;
	};

	struct ExecuteData final : public PassExecuteData
	{
		U32 BindingIndex = UINT32_MAX;
		Resource::PipelineStateGfx StateStencil;
		Resource::PipelineStateGfx StateRender;

		ExecuteData() = default;
		ZE_CLASS_MOVE(ExecuteData);
		virtual ~ExecuteData() = default;
	};

	constexpr bool Evaluate() noexcept { return true; } // TODO: check input data

	PassDesc GetDesc(PixelFormat formatRT, PixelFormat formatDS) noexcept;
	ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat formatRT, PixelFormat formatDS) noexcept;
	Expected<bool> Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept;
}