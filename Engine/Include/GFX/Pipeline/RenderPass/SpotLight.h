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
		ChainPool<std::vector<Resource::CBuffer>> ShadowBuffers;
		ChainPool<std::vector<Resource::CBuffer>> TransformBuffers;
		Resource::VertexBuffer VolumeVB;
		Resource::IndexBuffer VolumeIB;
	};

	inline void Clean(void* data) { delete reinterpret_cast<ExecuteData*>(data); }

	ExecuteData* Setup(Device& dev, RendererBuildData& buildData,
		PixelFormat formatColor, PixelFormat formatSpecular,
		PixelFormat formatShadow, PixelFormat formatShadowDepth);
	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData);
}