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
		xess_quality_settings_t Quality = XESS_QUALITY_SETTING_AA;
	};

	constexpr bool Evaluate() noexcept { return Settings::GetUpscaler() == UpscalerType::XeSS; }

	PassDesc GetDesc() noexcept;
	void Clean(Device& dev, void* data) noexcept;
	bool Update(Device& dev, ExecuteData& passData);
	void* Initialize(Device& dev, RendererPassBuildData& buildData);
	void Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData);
}