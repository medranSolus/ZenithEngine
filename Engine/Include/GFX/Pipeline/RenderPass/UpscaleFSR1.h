#pragma once
#include "GFX/Pipeline/PassDesc.h"
ZE_WARNING_PUSH
#include "FidelityFX/host/ffx_fsr1.h"
ZE_WARNING_POP

namespace ZE::GFX::Pipeline::RenderPass::UpscaleFSR1
{
	struct Resources
	{
		RID Color;
		RID Output;
	};

	struct ExecuteData final : public PassExecuteData
	{
		FfxFsr1Context Ctx = {};
		UInt2 DisplaySize = { 0, 0 };
		FfxFsr1QualityMode Quality = FFX_FSR1_QUALITY_MODE_ULTRA_QUALITY;
		bool SharpeningEnabled = true;
		float Sharpness = 0.8f;
		bool Initialized = false;

		ExecuteData() = default;
		ZE_CLASS_MOVE(ExecuteData);
		virtual ~ExecuteData();
	};

	constexpr bool Evaluate() noexcept { return Settings::Upscaler == UpscalerType::Fsr1; }

	PassDesc GetDesc(PixelFormat formatOutput) noexcept;
	Expected<UpdateOperation> Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData, PixelFormat formatOutput) noexcept;
	ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat formatOutput) noexcept;
	Expected<bool> Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept;
	void DebugUI(PassExecuteData* data) noexcept;
}