#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Resource/PipelineStateGfx.h"

namespace ZE::GFX::Pipeline::RenderPass::Lambertian
{
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
		bool MotionEnabled;
		bool ReactiveEnabled;
	};

	constexpr bool Evaluate() noexcept { return true; } // TODO: check input element count

	PassDesc GetDesc(PixelFormat formatDS, PixelFormat formatNormal, PixelFormat formatAlbedo,
		PixelFormat formatMaterialParams, PixelFormat formatMotion, PixelFormat formatReactive) noexcept;
	void Clean(Device& dev, void* data, GpuSyncStatus& syncStatus);
	UpdateStatus Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData, PixelFormat formatDS, PixelFormat formatNormal,
		PixelFormat formatAlbedo, PixelFormat formatMaterialParams, PixelFormat formatMotion, PixelFormat formatReactive);
	void* Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat formatDS, PixelFormat formatNormal,
		PixelFormat formatAlbedo, PixelFormat formatMaterialParams, PixelFormat formatMotion, PixelFormat formatReactive);
	void Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData);
}