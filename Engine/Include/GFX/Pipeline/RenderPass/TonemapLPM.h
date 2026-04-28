#pragma once
#include "GFX/Pipeline/PassDesc.h"
ZE_WARNING_PUSH
#include "FidelityFX/host/ffx_lpm.h"
ZE_WARNING_POP

namespace ZE::GFX::Pipeline::RenderPass::TonemapLPM
{
	struct Resources
	{
		RID Scene;
		RID RenderTarget;
	};

	struct ExecuteData final : public PassExecuteData
	{
		FfxLpmContext Ctx = {};
		FfxLpmColorSpace ColorSpace = FfxLpmColorSpace::FFX_LPM_ColorSpace_REC709;
		FfxLpmDisplayMode DisplayMode = FfxLpmDisplayMode::FFX_LPM_DISPLAYMODE_LDR;
		bool Shoulder = true;
		float SoftGap = 0.0f;
		float HdrMax = 6.0f;
		float Exposure = 3.1f;
		float Contrast = 0.2f;
		float ShoulderContrast = 1.0f;
		Float3 Saturation = { 0.0f, 0.0f, 0.0f };
		Float3 CrossTalk = { 1.0f, 0.5f, 1.0f / 32.0f };
		bool Initialized = false;

		ExecuteData() = default;
		ZE_CLASS_MOVE(ExecuteData);
		virtual ~ExecuteData();
	};

	constexpr bool Evaluate() noexcept { return Settings::Tonemapper == TonemapperType::LPM; }

	PassDesc GetDesc() noexcept;
	Expected<std::unique_ptr<ExecuteData>> Initialize(Device& dev, RendererPassBuildData& buildData) noexcept;
	Status Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept;
	void DebugUI(PassExecuteData* data) noexcept;
}