#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Resource/CBuffer.h"
#include "GFX/Resource/PipelineStateGfx.h"

namespace ZE::GFX::Pipeline::RenderPass::TonemapGT7
{
	// Defines the SDR reference white level used in our tone mapping [cd/m^2]
	constexpr float GT7_SDR_PAPER_WHITE = 250.0f;

#pragma pack(push, 1)
	struct TonemapParams
	{
		float ParamA = FLT_MAX;
		float ParamB = FLT_MAX;
		float ParamC = FLT_MAX;
		float MidPoint = 0.538f;
		float ToeStrength = 1.280f;
		float ShoulderTreshold = FLT_MAX;
		float FadeStart = 0.98f;
		float FadeEnd = 1.16f;
		float LuminanceTargetUCS = FLT_MAX;
		float LuminanceTarget = FLT_MAX;
		float BlendRatio = 0.6f;
		float CorrectionSDR = FLT_MAX;
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
		Resource::CBuffer ParamsBuffer;
		TonemapParams Params = {};
		float Exposure = 4.0f;
		float Alpha = 0.25f;
		float LinearSection = 0.444f;
		bool UseJzazbz = true;
		bool UpdatePso = false;
		bool UpdateData = false;

		ExecuteData() = default;
		ZE_CLASS_MOVE(ExecuteData);
		virtual ~ExecuteData() = default;
	};

	constexpr bool Evaluate() noexcept { return Settings::Tonemapper == TonemapperType::GranTurismo7; }

	PassDesc GetDesc(PixelFormat outputFormat) noexcept;
	Expected<UpdateOperation> Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData, PixelFormat outputFormat) noexcept;
	Expected<std::unique_ptr<ExecuteData>> Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat outputFormat) noexcept;
	Status Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept;
	void DebugUI(PassExecuteData* data) noexcept;
}