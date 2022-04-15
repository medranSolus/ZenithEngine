#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Pipeline/RendererBuildData.h"
#include "GFX/TransformBuffer.h"

namespace ZE::GFX::Pipeline::RenderPass::ShadowMapCube
{
	constexpr U64 BUFFER_SHRINK_STEP = 2;

	// Indicates that material of the geometry can be processed in depth pre-pass
	struct Solid { Resource::DynamicBufferAlloc Transform; };
	// Indicates that material of the geometry cannot be processed in depth pre-pass
	struct Transparent {};

	struct Resources
	{
		RID RenderTarget;
		RID Depth;
	};

	struct ExecuteData
	{
		U32 BindingIndex;
		Resource::PipelineStateGfx StateDepth;
		Ptr<Resource::PipelineStateGfx> StatesSolid;
		Ptr<Resource::PipelineStateGfx> StatesTransparent;
		Matrix Projection;
	};

	void Clean(ExecuteData& data);
	void Setup(Device& dev, RendererBuildData& buildData,
		ExecuteData& passData, PixelFormat formatDS, PixelFormat formatRT);
	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData,
		ExecuteData& data, const Resources& ids, const Float3& lightPos, float lightVolume);
}