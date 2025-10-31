#pragma once
#include "ShadowMap.h"

namespace ZE::GFX::Pipeline::RenderPass::SpotLight
{
	struct Resources
	{
		RID GBufferDepth;
		RID GBufferNormal;
		RID GBufferAlbedo;
		RID GBufferMaterialParams;
		RID Lighting;
		RID ShadowMap;
		RID ShadowMapDepth;
	};

	struct ExecuteData
	{
		ShadowMap::ExecuteData ShadowData;
		U32 BindingIndex;
		Resource::PipelineStateGfx State;
		Resource::Mesh VolumeMesh;
	};

	constexpr bool Evaluate() noexcept { return true; } // TODO: Check input data

	PassDesc GetDesc(PixelFormat formatLighting, PixelFormat formatShadow, PixelFormat formatShadowDepth) noexcept;
	void Clean(Device& dev, void* data, GpuSyncStatus& syncStatus);
	void* Initialize(Device& dev, RendererPassBuildData& buildData,
		PixelFormat formatLighting, PixelFormat formatShadow, PixelFormat formatShadowDepth);
	bool Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData);
}