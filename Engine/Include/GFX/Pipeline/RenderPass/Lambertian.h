#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Pipeline/RendererBuildData.h"
#include "GFX/Pipeline/Info/World.h"
#include "GFX/TransformBuffer.h"

namespace ZE::GFX::Pipeline::RenderPass::Lambertian
{
	constexpr U64 BUFFER_SHRINK_STEP = 2;

	struct Resources
	{
		RID DepthStencil;
		RID Color;
		RID Normal;
		RID Specular;
	};

	struct Data
	{
		Info::World& World;
		U32 BindingIndex;
		Resource::PipelineStateGfx StateDepth;
		Resource::PipelineStateGfx StateNormal;
		std::vector<Resource::CBuffer> TransformBuffers;
	};

	inline void Clean(void* data) { delete reinterpret_cast<Data*>(data); }

	Data* Setup(Device& dev, RendererBuildData& buildData, Info::World& worldData, PixelFormat formatDS,
		PixelFormat formatColor, PixelFormat formatNormal, PixelFormat formatSpecular);
	void Execute(RendererExecuteData& renderData, PassData& passData);
}