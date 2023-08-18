#pragma once
#include "ShadowMap.h"

namespace ZE::GFX::Pipeline::RenderPass::SpotLight
{
	constexpr U64 BUFFER_SHRINK_STEP = 1;

	struct Resources
	{
		RID GBufferNormal;
		RID GBufferSpecular;
		RID GBufferDepth;
		RID Color;
		RID Specular;
		RID ShadowMap;
		RID ShadowMapDepth;
	};

	struct ExecuteData
	{
		ShadowMap::ExecuteData ShadowData;
		U32 BindingIndex;
		Resource::PipelineStateGfx State;
		Resource::Mesh VolumeMesh;
	};

	void Clean(Device& dev, void* data) noexcept;
	ExecuteData* Setup(Device& dev, RendererBuildData& buildData,
		PixelFormat formatColor, PixelFormat formatSpecular,
		PixelFormat formatShadow, PixelFormat formatShadowDepth);
	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData);
}