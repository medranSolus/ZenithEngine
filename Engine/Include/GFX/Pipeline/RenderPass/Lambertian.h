#pragma once
#include "GFX/Pipeline/RenderLevel.h"
#include "GFX/Pipeline/RendererBuildData.h"
#include "GFX/TransformBuffer.h"

namespace ZE::GFX::Pipeline::RenderPass::Lambertian
{
	constexpr U64 BUFFER_SHRINK_STEP = 2;

	// Indicates that entity is inside view frustum
	struct InsideFrustumSolid { Resource::DynamicBufferAlloc Transform; };
	// Indicates that entity is inside view frustum and is not opaque
	struct InsideFrustumNotSolid {};

	struct Resources
	{
		RID DepthStencil;
		RID Color;
		RID Normal;
		RID Specular;
	};

	struct ExecuteData
	{
		U32 BindingIndex;
		Resource::PipelineStateGfx StateDepth;
		Ptr<Resource::PipelineStateGfx> StatesSolid;
		Ptr<Resource::PipelineStateGfx> StatesTransparent;
	};

	void Clean(void* data);
	ExecuteData* Setup(Device& dev, RendererBuildData& buildData, PixelFormat formatDS,
		PixelFormat formatColor, PixelFormat formatNormal, PixelFormat formatSpecular);
	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData);
}