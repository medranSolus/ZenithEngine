#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Resource/PipelineStateGfx.h"

namespace ZE::GFX::Pipeline::RenderPass::TonemapReinhardX
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
		Float3 Params = { 1.5f, 1.0f, 2.0f }; // Exposure | Offset | MaxWhite
		TonemapperType CurrentTonemapper = TonemapperType::None;

		ExecuteData() = default;
		ZE_CLASS_MOVE(ExecuteData);
		virtual ~ExecuteData() = default;
	};

	constexpr bool Evaluate() noexcept { return Settings::Tonemapper == TonemapperType::ReinhardExtended || Settings::Tonemapper == TonemapperType::ReinhardLumaPreserveWhite; }

	PassDesc GetDesc(PixelFormat outputFormat) noexcept;
	Expected<UpdateOperation> Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData, PixelFormat outputFormat) noexcept;
	Expected<std::unique_ptr<ExecuteData>> Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat outputFormat) noexcept;
	Status Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept;
	void DebugUI(PassExecuteData* data) noexcept;
}