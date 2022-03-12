#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Pipeline/RendererBuildData.h"
#include "GFX/TransformBuffer.h"

namespace ZE::GFX::Pipeline::RenderPass::ShadowMapCube
{
	constexpr U64 BUFFER_SHRINK_STEP = 2;

	struct Resources
	{
		RID RenderTarget;
		RID Depth;
	};

	struct ExecuteData
	{
		U32 BindingIndex;
		Resource::PipelineStateGfx StateDepth;
		Resource::PipelineStateGfx StateNormal;
		Resource::CBuffer ViewBuffer;
		std::vector<Resource::CBuffer> TransformBuffers;
		Matrix Projection;
	};

	void Setup(Device& dev, RendererBuildData& buildData,
		ExecuteData& passData, PixelFormat formatDS, PixelFormat formatRT);
	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData,
		ExecuteData& data, const Resources& ids, const Float3& lightPos);
}