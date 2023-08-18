#pragma once
#include "GFX/Resource/Texture/Pack.h"
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Pipeline/RendererBuildData.h"

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
		Resource::Texture::Pack SkyTexture;
		Resource::Mesh MeshData;
	};

	void Clean(Device& dev, void* data) noexcept;
	ExecuteData* Setup(Device& dev, RendererBuildData& buildData, PixelFormat formatRT,
		PixelFormat formatDS, const std::string& cubemapPath, const std::string& cubemapExt);
	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData);
}