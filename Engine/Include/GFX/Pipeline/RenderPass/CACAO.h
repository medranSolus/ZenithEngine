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

	struct ExecuteData
	{
		FfxCacaoContext Ctx;
		FfxCacaoSettings Settings = FFX_CACAO_DEFAULT_SETTINGS;
		UInt2 RenderSize = { 0, 0 };
	};

	constexpr bool Evaluate() noexcept { return Settings::AmbientOcclusionType == AOType::CACAO; }

	PassDesc GetDesc() noexcept;
	void Clean(Device& dev, void* data, GpuSyncStatus& syncStatus);
	UpdateStatus Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData, bool firstUpdate = false);
	void* Initialize(Device& dev, RendererPassBuildData& buildData);
	void Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData);
	void DebugUI(void* data) noexcept;
}