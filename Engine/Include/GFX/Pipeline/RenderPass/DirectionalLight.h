#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Pipeline/RendererBuildData.h"
#include "GFX/Pipeline/Info/World.h"

namespace ZE::GFX::Pipeline::RenderPass::DirectionalLight
{
	struct Resources
	{
		RID GBufferNormal;
		RID GBufferSpecular;
		RID GBufferDepth;
		RID ShadowMap;
		RID ShadowMapDepth;
		RID Color;
		RID Specular;
	};

	struct Data
	{
		Info::World& World;
		U32 BindingIndex;
		Resource::PipelineStateGfx State;
	};

	inline void Clean(void* data) { delete reinterpret_cast<Data*>(data); }

	Data* Setup(Device& dev, RendererBuildData& buildData, Info::World& worldData,
		PixelFormat formatColor, PixelFormat formatSpecular,
		PixelFormat formatShadow, PixelFormat formatShadowDepth);
	void Execute(RendererExecuteData& renderData, PassData& passData);
}