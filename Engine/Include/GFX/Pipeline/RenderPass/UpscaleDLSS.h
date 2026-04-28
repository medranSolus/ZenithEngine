#pragma once
#if _ZE_NGX_ENABLED
#	include "GFX/Pipeline/PassDesc.h"
ZE_WARNING_PUSH
#	include "nvsdk_ngx_params.h"
ZE_WARNING_POP

namespace ZE::GFX::Pipeline::RenderPass::UpscaleDLSS
{
	struct Resources
	{
		RID Color;
		RID Depth;
		RID MotionVectors;
		RID Output;
	};

	struct ExecuteData final : public PassExecuteData
	{
		UInt2 DisplaySize = { 0, 0 };
		NVSDK_NGX_PerfQuality_Value Quality = NVSDK_NGX_PerfQuality_Value_DLAA;
		bool SharpeningEnabled = true;
		float Sharpness = 0.0f;
		NVSDK_NGX_Parameter* NgxParam = nullptr;
		NVSDK_NGX_Handle* DlssHandle = nullptr;

		ExecuteData() = default;
		ZE_CLASS_MOVE(ExecuteData);
		virtual ~ExecuteData();
	};

	constexpr bool Evaluate() noexcept { return Settings::Upscaler == UpscalerType::DLSS; }

	PassDesc GetDesc() noexcept;
	Expected<UpdateOperation> Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData) noexcept;
	Expected<std::unique_ptr<ExecuteData>> Initialize(Device& dev, RendererPassBuildData& buildData) noexcept;
	Status Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept;
	void DebugUI(PassExecuteData* data) noexcept;
}
#endif