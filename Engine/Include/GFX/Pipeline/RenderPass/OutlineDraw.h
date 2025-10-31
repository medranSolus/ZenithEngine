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

	struct ExecuteData
	{
		U32 BindingIndex;
		Resource::PipelineStateGfx StateStencil;
		Resource::PipelineStateGfx StateRender;
	};

	constexpr bool Evaluate() noexcept { return true; } // TODO: check input data

	PassDesc GetDesc(PixelFormat formatRT, PixelFormat formatDS) noexcept;
	void Clean(Device& dev, void* data, GpuSyncStatus& syncStatus);
	void* Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat formatRT, PixelFormat formatDS);
	bool Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData);
}