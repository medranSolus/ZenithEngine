#pragma once
#include "GFX/External/HbaoCtx.h"
#include "GFX/Resource/PipelineStateGfx.h"
#include "GFX/Pipeline/PassDesc.h"

namespace ZE::GFX::Pipeline::RenderPass::HBAO
{
	struct Resources
	{
		RID Depth;
		RID Normal;
		RID InternalNormals;
		RID InternalAO;
		RID AO;
	};

	struct ExecuteData final : public PassExecuteData
	{
		External::HbaoCtx Ctx;
		GFSDK_SSAO_Parameters Params = {};
		UInt2 RenderSize = { 0, 0 };
		U32 BindingIndex = UINT32_MAX;
		Resource::PipelineStateGfx UnpackNormals;

		ExecuteData() = default;
		ZE_CLASS_MOVE(ExecuteData);
		virtual ~ExecuteData() = default;
	};

	constexpr bool Evaluate() noexcept { return Settings::AmbientOcclusionType == AOType::HBAO; }

	PassDesc GetDesc(PixelFormat internalNormalsFormat) noexcept;
	Expected<UpdateOperation> Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData) noexcept;
	ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat internalNormalsFormat) noexcept;
	Expected<bool> Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept;
	void DebugUI(PassExecuteData* data) noexcept;
}