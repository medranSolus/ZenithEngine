#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Resource/PipelineStateGfx.h"

namespace ZE::GFX::Pipeline::RenderPass::HorizontalBlur
{
	struct Resources
	{
		RID Outline;
		RID RenderTarget;
	};

	struct ExecuteData
	{
		U32 BindingIndex;
		Resource::PipelineStateGfx State;
	};

	PassDesc GetDesc(PixelFormat formatRT) noexcept;
	void Clean(Device& dev, void* data, GpuSyncStatus& syncStatus);
	void* Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat formatRT);
	void Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData);
}