#pragma once
#include "GFX/Pipeline/PassDesc.h"
ZE_WARNING_PUSH
#include "FidelityFX/host/ffx_cacao.h"
ZE_WARNING_POP

namespace ZE::GFX::Pipeline::RenderPass::CACAO
{
	struct Resources
	{
		RID Depth;
		RID Normal;
		RID AO;
	};

	struct ExecuteData final : public PassExecuteData
	{
		FfxCacaoContext Ctx = {};
		FfxCacaoSettings Settings = FFX_CACAO_DEFAULT_SETTINGS;
		UInt2 RenderSize = { 0, 0 };
		bool Initialized = false;

		ExecuteData() = default;
		ZE_CLASS_MOVE(ExecuteData);
		virtual ~ExecuteData();
	};

	constexpr bool Evaluate() noexcept { return Settings::AmbientOcclusionType == AOType::CACAO; }

	PassDesc GetDesc() noexcept;
	Expected<UpdateOperation> Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData) noexcept;
	ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData) noexcept;
	Expected<bool> Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept;
	void DebugUI(PassExecuteData* data) noexcept;
}