#pragma once
#include "GFX/Pipeline/PassDesc.h"
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
		RID Normal;
		RID Albedo;
		RID MaterialParams;
		RID MotionVectors;
		RID ReactiveMask;
	};

	struct ExecuteData
	{
		U32 BindingIndex;
		Resource::PipelineStateGfx StateDepth;
		Ptr<Resource::PipelineStateGfx> StatesSolid;
		Ptr<Resource::PipelineStateGfx> StatesTransparent;
	};

	void Clean(Device& dev, void* data) noexcept;
	ExecuteData* Setup(Device& dev, RendererBuildData& buildData, PixelFormat formatDS, PixelFormat formatNormal,
		PixelFormat formatAlbedo, PixelFormat formatMaterialParams, PixelFormat formatMotion, PixelFormat formatReactive);
	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData);
}