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
		float ParamA;
		float ParamB;
		float ParamC;
		float MidPoint = 0.538f;
		float ToeStrength = 1.280f;
		float ShoulderTreshold;
		float FadeStart = 0.98f;
		float FadeEnd = 1.16f;
		float LuminanceTargetUCS;
		float LuminanceTarget;
		float BlendRatio = 0.6f;
		float CorrectionSDR;
	};
#pragma pack(pop)

	struct Resources
	{
		RID Scene;
		RID RenderTarget;
	};

	struct ExecuteData
	{
		U32 BindingIndex;
		Resource::PipelineStateGfx State;
		Resource::CBuffer ParamsBuffer;
		TonemapParams Params = {};
		float Exposure = 4.0f;
		float Alpha = 0.25f;
		float LinearSection = 0.444f;
		bool UseJzazbz = true;
		bool UpdatePso = false;
		bool UpdateData = false;
	};

	constexpr bool Evaluate() noexcept { return Settings::Tonemapper == TonemapperType::GranTurismo7; }

	PassDesc GetDesc(PixelFormat outputFormat) noexcept;
	void Clean(Device& dev, void* data, GpuSyncStatus& syncStatus);
	UpdateStatus Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData, PixelFormat outputFormat, bool firstCall = false);
	void* Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat outputFormat);
	bool Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData);
	void DebugUI(void* data) noexcept;
}