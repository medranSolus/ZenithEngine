#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Resource/PipelineStateGfx.h"

namespace ZE::GFX::Pipeline::RenderPass::TonemapVDR
{
#pragma pack(push, 1)
	struct TonemapParams
	{
		float Exposure = 2.6f;
		float Contrast = 1.3f;
		float B = 0.0f;
		float C = 0.0f;
		float Shoulder = 0.995f;
	};
#pragma pack(pop)

	struct Resources
	{
		RID Scene;
		RID RenderTarget;
	};

	struct ExecuteData final : public PassExecuteData
	{
		U32 BindingIndex = UINT32_MAX;
		Resource::PipelineStateGfx State;
		TonemapParams Params = {};
		float MidIn = 0.18f;
		float MidOut = 0.18f;
		float MaxRadiance = 64.0f;

		ExecuteData() = default;
		ZE_CLASS_MOVE(ExecuteData);
		virtual ~ExecuteData() = default;
	};

	constexpr bool Evaluate() noexcept { return Settings::Tonemapper == TonemapperType::FilmicVDR; }

	PassDesc GetDesc(PixelFormat outputFormat) noexcept;
	Expected<std::unique_ptr<ExecuteData>> Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat outputFormat) noexcept;
	Status Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept;
	void DebugUI(PassExecuteData* data) noexcept;
}