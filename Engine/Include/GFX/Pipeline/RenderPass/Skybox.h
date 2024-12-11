#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Resource/Texture/Pack.h"
#include "GFX/Resource/PipelineStateGfx.h"

namespace ZE::GFX::Pipeline::RenderPass::Skybox
{
	struct Resources
	{
		RID RenderTarget;
		RID DepthStencil;
	};

	struct ExecuteData
	{
		U32 BindingIndex;
		Resource::PipelineStateGfx State;
		// TODO: maybe move out as RID if environment map would use it too
		Resource::Texture::Pack SkyTexture;
		Resource::Mesh MeshData;
	};

	PassDesc GetDesc(PixelFormat formatRT, PixelFormat formatDS,
		const std::string& cubemapPath, const std::string& cubemapExt) noexcept;
	void Clean(Device& dev, void* data, GpuSyncStatus& syncStatus);
	void* CopyInitData(void* data) noexcept;
	void FreeInitData(void* data) noexcept;
	void* Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat formatRT,
		PixelFormat formatDS, const std::string& cubemapPath, const std::string& cubemapExt);
	void Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData);
}