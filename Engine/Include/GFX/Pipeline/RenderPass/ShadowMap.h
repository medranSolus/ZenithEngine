#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Pipeline/RendererBuildData.h"
#include "GFX/TransformBuffer.h"

namespace ZE::GFX::Pipeline::RenderPass::ShadowMap
{
	constexpr U64 BUFFER_SHRINK_STEP = 2;

	// Indicates that entity is inside view frustum
	struct InsideFrustumSolid {};
	// Indicates that entity is inside view frustum and is not opaque
	struct InsideFrustumNotSolid {};

	struct Resources
	{
		RID RenderTarget;
		RID Depth;
	};

	struct ExecuteData
	{
		U32 BindingIndex;
		Resource::PipelineStateGfx StateDepth;
		Resource::PipelineStateGfx StateSolid;
		Resource::PipelineStateGfx StateTransparent;
		std::vector<Resource::CBuffer> TransformBuffers;
		Matrix Projection;
		// Number of entities that were previously used in computing shadow map,
		// have to be zeroed once per frame
		U64 PreviousEntityCount = 0;
	};

	void Setup(Device& dev, RendererBuildData& buildData, ExecuteData& passData,
		PixelFormat formatDS, PixelFormat formatRT, Matrix&& projection);
	Matrix Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData,
		ExecuteData& data, const Resources& ids, const Float3& lightPos, const Float3& lightDir);
}