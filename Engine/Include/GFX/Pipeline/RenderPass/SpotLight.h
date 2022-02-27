#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Pipeline/RendererBuildData.h"
#include "GFX/Pipeline/Info/World.h"
#include "GFX/TransformBuffer.h"

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
		Info::World& World;
		U32 BindingIndex;
		Resource::PipelineStateGfx State;
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