#pragma once
#include "ShadowMapCube.h"

namespace ZE::GFX::Pipeline::RenderPass::PointLight
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
		ShadowMapCube::ExecuteData ShadowData;
		U32 BindingIndex;
		Resource::PipelineStateGfx State;
		Resource::Mesh VolumeMesh;
	};

	constexpr bool Evaluate() noexcept { return true; } // TODO: Check input data

	PassDesc GetDesc(PixelFormat formatLighting, PixelFormat formatShadow, PixelFormat formatShadowDepth) noexcept;
	void Clean(Device& dev, void* data) noexcept;
	void* Initialize(Device& dev, RendererPassBuildData& buildData,
		PixelFormat formatLighting, PixelFormat formatShadow, PixelFormat formatShadowDepth);
	void Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData);
}