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

	struct Data
	{
		ShadowMap::Data ShadowData;
		U32 BindingIndex;
		Resource::PipelineStateGfx State;
		std::vector<Resource::CBuffer> ShadowBuffers;
		std::vector<Resource::CBuffer> TransformBuffers;
		Resource::VertexBuffer VolumeVB;
		Resource::IndexBuffer VolumeIB;
	};

	inline void Clean(void* data) { delete reinterpret_cast<Data*>(data); }

	Data* Setup(Device& dev, RendererBuildData& buildData, Info::World& worldData,
		PixelFormat formatColor, PixelFormat formatSpecular,
		PixelFormat formatShadow, PixelFormat formatShadowDepth);
	void Execute(RendererExecuteData& renderData, PassData& passData);
}