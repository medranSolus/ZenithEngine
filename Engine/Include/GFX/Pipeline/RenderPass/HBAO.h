#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/External/HbaoCtx.h"

namespace ZE::GFX::Pipeline::RenderPass::HBAO
{
	struct Resources
	{
		RID Depth;
		RID Normal;
		RID AO;
	};

	struct ExecuteData final : public PassExecuteData
	{
		External::HbaoCtx Ctx;
		GFSDK_SSAO_Parameters Params = {};
		UInt2 RenderSize = { 0, 0 };

		ExecuteData() = default;
		ZE_CLASS_MOVE(ExecuteData);
		virtual ~ExecuteData() = default;
	};

	constexpr bool Evaluate() noexcept { return Settings::AmbientOcclusionType == AOType::HBAO; }

	PassDesc GetDesc() noexcept;
	Expected<UpdateOperation> Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData) noexcept;
	ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData) noexcept;
	Expected<bool> Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept;
	void DebugUI(PassExecuteData* data) noexcept;
}