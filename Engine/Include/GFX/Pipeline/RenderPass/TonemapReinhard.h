#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Resource/PipelineStateGfx.h"

namespace ZE::GFX::Pipeline::RenderPass::TonemapReinhard
{
	struct Resources
	{
		RID Scene;
		RID RenderTarget;
	};

	struct ExecuteData final : public PassExecuteData
	{
		U32 BindingIndex = UINT32_MAX;
		Resource::PipelineStateGfx State;
		Float2 Params = { 1.5f, 1.0f }; // Exposure | Offset
		TonemapperType CurrentTonemapper = TonemapperType::None;

		ExecuteData() = default;
		ZE_CLASS_MOVE(ExecuteData);
		virtual ~ExecuteData() = default;
	};

	constexpr bool Evaluate() noexcept { return Settings::Tonemapper == TonemapperType::Reinhard || Settings::Tonemapper == TonemapperType::ReinhardLuma || Settings::Tonemapper == TonemapperType::ReinhardLumaJodie; }

	PassDesc GetDesc(PixelFormat outputFormat) noexcept;
	Expected<UpdateOperation> Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData, PixelFormat outputFormat) noexcept;
	ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat outputFormat) noexcept;
	Expected<bool> Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept;
	void DebugUI(PassExecuteData* data) noexcept;
}