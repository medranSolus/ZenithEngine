#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Resource/PipelineStateGfx.h"

namespace ZE::GFX::Pipeline::RenderPass::Wireframe
{
	// Indicates that entity is inside view frustum
	struct InsideFrustum {};

	struct Resources
	{
		RID RenderTarget;
		RID DepthStencil;
	};

	struct ExecuteData
	{
		U32 BindingIndex;
		Resource::PipelineStateGfx State;
	};

	constexpr bool Evaluate() noexcept { return true; } // TODO: check input element count

	PassDesc GetDesc(PixelFormat formatRT, PixelFormat formatDS) noexcept;
	void Clean(Device& dev, void* data, GpuSyncStatus& syncStatus);
	void* Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat formatRT, PixelFormat formatDS);
	void Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData);
}