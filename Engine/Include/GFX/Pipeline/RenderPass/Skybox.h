#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Resource/PipelineStateGfx.h"

namespace ZE::GFX::Pipeline::RenderPass::Skybox
{
	struct Resources
	{
		RID RenderTarget;
		RID DepthStencil;
		RID Skybox;
	};

	struct ExecuteData
	{
		U32 BindingIndex;
		Resource::PipelineStateGfx State;
		Resource::Mesh MeshData;
	};

	PassDesc GetDesc(PixelFormat formatRT, PixelFormat formatDS) noexcept;
	void Clean(Device& dev, void* data, GpuSyncStatus& syncStatus);
	void* Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat formatRT, PixelFormat formatDS);
	bool Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData);
}