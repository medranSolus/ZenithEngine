#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Resource/PipelineStateGfx.h"

namespace ZE::GFX::Pipeline::RenderPass::LightCombine
{
	struct Resources
	{
		RID DirectLight;
		RID SSAO;
		RID SSR;
		RID IrrMap;
		RID EnvMap;
		RID BrdfLUT;
		RID GBufferDepth;
		RID GBufferNormal;
		RID GBufferAlbedo;
		RID GBufferMaterialParams;
		RID RenderTarget;
	};

	struct ExecuteData final : public PassExecuteData
	{
		U32 BindingIndex = UINT32_MAX;
		Resource::PipelineStateGfx State;
		bool AmbientOcclusionEnabled = false;
		bool IBLState = false;
		bool SSRState = false;

		ExecuteData() = default;
		ZE_CLASS_MOVE(ExecuteData);
		virtual ~ExecuteData() = default;
	};

	PassDesc GetDesc(PixelFormat outputFormat) noexcept;
	Expected<UpdateOperation> Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData, PixelFormat outputFormat) noexcept;
	ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat outputFormat) noexcept;
	Expected<bool> Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept;
}