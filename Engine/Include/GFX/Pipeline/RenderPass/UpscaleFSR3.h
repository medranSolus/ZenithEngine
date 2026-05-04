#pragma once
#include "GFX/Pipeline/PassDesc.h"
ZE_WARNING_PUSH
#include "FidelityFX/host/ffx_fsr3upscaler.h"
ZE_WARNING_POP

namespace ZE::GFX::Pipeline::RenderPass::UpscaleFSR3
{
	struct Resources
	{
		RID Color;
		RID Depth;
		RID MotionVectors;
		RID ReactiveMask;
		RID Output;
		RID DilatedDepth;
		RID DilatedMotion;
		RID PrevNearDepth;
	};

	struct ExecuteData final : public PassExecuteData
	{
		FfxFsr3UpscalerContext Ctx = {};
		UInt2 DisplaySize = { 0, 0 };
		FfxFsr3UpscalerQualityMode Quality = FFX_FSR3UPSCALER_QUALITY_MODE_NATIVEAA;
		bool SharpeningEnabled = true;
		float Sharpness = 0.7f;
		bool Initialized = false;

		ExecuteData() = default;
		ZE_CLASS_MOVE(ExecuteData);
		virtual ~ExecuteData();
	};

	constexpr bool Evaluate() noexcept { return Settings::Upscaler == UpscalerType::Fsr3; }

	PassDesc GetDesc() noexcept;
	Expected<UpdateOperation> Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData) noexcept;
	ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData) noexcept;
	Expected<bool> Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept;
	void DebugUI(PassExecuteData* data) noexcept;
}