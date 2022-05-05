#pragma once
#include "GFX/Pipeline/RenderLevel.h"
#include "GFX/Pipeline/RendererBuildData.h"
#include "GFX/Resource/Texture/Pack.h"
#include "GFX/Resource/IndexBuffer.h"
#include "GFX/Resource/VertexBuffer.h"

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
		Resource::VertexBuffer VertexBuffer;
		Resource::IndexBuffer IndexBuffer;
	};

	inline void Clean(void* data) { delete reinterpret_cast<ExecuteData*>(data); }

	ExecuteData* Setup(Device& dev, RendererBuildData& buildData, PixelFormat formatRT,
		PixelFormat formatDS, const std::string& cubemapPath, const std::string& cubemapExt);
	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData);
}