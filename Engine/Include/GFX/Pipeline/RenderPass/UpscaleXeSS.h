#pragma once
#include "GFX/Pipeline/PassDesc.h"
ZE_WARNING_PUSH
#include "xess/xess.h"
ZE_WARNING_POP

namespace ZE::GFX::Pipeline::RenderPass::UpscaleXeSS
{
	struct Resources
	{
		RID Color;
		RID Depth;
		RID MotionVectors;
		RID ResponsiveMask;
		RID Output;
	};

	struct ExecuteData
	{
		UInt2 DisplaySize = { 0, 0 };
		xess_quality_settings_t Quality = XESS_QUALITY_SETTING_ULTRA_QUALITY;
	};

	constexpr bool Evaluate() noexcept { return Settings::GetUpscaler() == UpscalerType::XeSS; }
	constexpr void Clean(Device& dev, void* data) noexcept { delete reinterpret_cast<ExecuteData*>(data); }

	PassDesc GetDesc() noexcept;
	bool Update(Device& dev, ExecuteData& passData);
	void* Initialize(Device& dev, RendererPassBuildData& buildData);
	void Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData);
}