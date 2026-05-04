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

	struct ExecuteData final : public PassExecuteData
	{
		ShadowMapCube::ExecuteData ShadowData = {};
		U32 BindingIndex = UINT32_MAX;
		Resource::PipelineStateGfx State;
		Resource::Mesh VolumeMesh;

		ExecuteData() = default;
		ZE_CLASS_MOVE(ExecuteData);
		virtual ~ExecuteData() = default;
	};

	constexpr bool Evaluate() noexcept { return true; } // TODO: Check input data

	PassDesc GetDesc(PixelFormat formatLighting, PixelFormat formatShadow, PixelFormat formatShadowDepth) noexcept;
	ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData,
		PixelFormat formatLighting, PixelFormat formatShadow, PixelFormat formatShadowDepth) noexcept;
	Expected<bool> Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept;
}