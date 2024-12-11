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

	struct ExecuteData
	{
		U32 BlockHeight;
		U32 BindingIndex;
		Resource::PipelineStateCompute StateUpscale;
		Resource::Texture::Pack Coefficients;
		bool Float16Support;
		UInt2 DisplaySize = { 0, 0 };
		NISQualityMode Quality = NISQualityMode::MegaQuality;
		float Sharpness = 0.5f;
	};

	constexpr bool Evaluate() noexcept { return Settings::Upscaler == UpscalerType::NIS; }

	PassDesc GetDesc() noexcept;
	void Clean(Device& dev, void* data, GpuSyncStatus& syncStatus);
	UpdateStatus Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData);
	void* Initialize(Device& dev, RendererPassBuildData& buildData);
	void Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData);
}