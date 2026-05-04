#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Resource/PipelineStateGfx.h"

namespace ZE::GFX::Pipeline::RenderPass::VerticalBlur
{
	struct Resources
	{
		RID OutlineBlur;
		RID RenderTarget;
		RID DepthStencil;
	};

	struct ExecuteData final : public PassExecuteData
	{
		U32 BindingIndex = UINT32_MAX;
		Resource::PipelineStateGfx State;

		ExecuteData() = default;
		ZE_CLASS_MOVE(ExecuteData);
		virtual ~ExecuteData() = default;
	};

	PassDesc GetDesc(PixelFormat formatRT, PixelFormat formatDS) noexcept;
	ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat formatRT, PixelFormat formatDS) noexcept;
	Expected<bool> Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept;
}