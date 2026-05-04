#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Resource/Texture/Pack.h"
#include "GFX/Resource/PipelineStateCompute.h"

namespace ZE::GFX::Pipeline::RenderPass::UpscaleNIS
{
	struct Resources
	{
		RID Color;
		RID Output;
	};

	struct ExecuteData final : public PassExecuteData
	{
		U32 BlockHeight = UINT32_MAX;
		U32 BindingIndex = UINT32_MAX;
		Resource::PipelineStateCompute StateUpscale;
		Resource::Texture::Pack Coefficients;
		bool Float16Support = false;
		UInt2 DisplaySize = { 0, 0 };
		NISQualityMode Quality = NISQualityMode::MegaQuality;
		bool SharpeningEnabled = true;
		float Sharpness = 0.5f;

		ExecuteData() = default;
		ZE_CLASS_MOVE(ExecuteData);
		virtual ~ExecuteData() { Settings::RenderSize = Settings::DisplaySize; }
	};

	constexpr bool Evaluate() noexcept { return Settings::Upscaler == UpscalerType::NIS; }

	PassDesc GetDesc() noexcept;
	Expected<UpdateOperation> Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData) noexcept;
	ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData) noexcept;
	Expected<bool> Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept;
	void DebugUI(PassExecuteData* data) noexcept;
}