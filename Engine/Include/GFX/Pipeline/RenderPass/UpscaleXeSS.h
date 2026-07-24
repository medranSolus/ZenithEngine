#pragma once
#if _ZE_XESS_ENABLED
#	include "GFX/Pipeline/PassDesc.h"
ZE_WARNING_PUSH
#	include "xess/xess.h"
ZE_WARNING_POP

namespace ZE::GFX::Pipeline::RenderPass::UpscaleXeSS
{
	struct Resources
	{
		RID Color;
		RID Depth;
		RID MotionVectors;
		RID ResponsiveMask;
		RID AliasableTexture;
		RID AliasableBuffer;
		RID Output;
	};

	struct ExecuteData final : public PassExecuteData
	{
		UInt2 DisplaySize = { 0, 0 };
		xess_quality_settings_t Quality = XESS_QUALITY_SETTING_AA;

		ExecuteData() = default;
		ZE_CLASS_MOVE(ExecuteData);
		virtual ~ExecuteData();
	};

	constexpr bool Evaluate() noexcept { return Settings::Upscaler == UpscalerType::XeSS; }

	Expected<UInt2> AliasableMemoryQuery(Device& dev, const FrameResourceDesc& desc) noexcept;
	PassDesc GetDesc() noexcept;
	Expected<UpdateOperation> Update(Device& dev, ExecuteData& passData) noexcept;
	ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData) noexcept;
	Expected<bool> Prepare(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept;
	Expected<bool> Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept;
	void DebugUI(PassExecuteData* data) noexcept;
}
#endif