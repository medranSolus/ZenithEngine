#pragma once
#include "DataPBR.h"
#include "ParamsPBR.h"
#include "RenderGraph.h"
#include "XeGTAOSettings.h"
ZE_WARNING_PUSH
#include "FidelityFX/host/ffx_cacao.h"
ZE_WARNING_POP

namespace ZE::GFX::Pipeline
{
	// Physically Based Renderer
	class RendererPBR final : public RenderGraph
	{
		DataPBR settingsData;
		CameraPBR dynamicData;
		float blurSigma = 0.0f;
		union
		{
			XeGTAOSettings xegtao;
			FfxCacaoSettings cacao;
		} ssaoSettings;
		Data::Projection currentProjectionData = {};
		Float4x4 currentProjection = {};
		Float4 cameraRotation = {};
		float sharpness = 0.5f;

		static void SetupRenderSlots(RendererBuildData& buildData) noexcept;

		constexpr void SetupBlurKernel() noexcept;
		constexpr void SetupBlurIntensity() noexcept;
		constexpr void SetupBlurData(U32 width, U32 height) noexcept;
		constexpr void SetupXeGTAOQuality() noexcept;
		constexpr void SetupSSAOData(U32 width, U32 height) noexcept;

	public:
		RendererPBR() noexcept : RenderGraph(this, &settingsData, &dynamicData, sizeof(CameraPBR)), ssaoSettings({}) {}
		ZE_CLASS_DELETE(RendererPBR);
		virtual ~RendererPBR() = default;

		constexpr XeGTAOSettings& GetXeGTAOSettings() noexcept { ZE_ASSERT(Settings::GetAOType() == AOType::XeGTAO, "XeGTAO is not active!"); return ssaoSettings.xegtao; }
		constexpr FfxCacaoSettings& GetCacaoSettings() noexcept { ZE_ASSERT(Settings::GetAOType() == AOType::CACAO, "CACAO is not active!"); return ssaoSettings.cacao; }

		constexpr const Data::Projection& GetProjectionData() const noexcept { return currentProjectionData; }
		constexpr const Float4x4& GetProjection() const noexcept { return currentProjection; }
		constexpr const Float4& GetCameraRotation() const noexcept { return cameraRotation; }
		constexpr float GetSharpness() const noexcept { return sharpness; }

		void Init(Device& dev, CommandList& mainList, const ParamsPBR& params);

		// Need to be called when data in parameters changed (also after creation of renderer)
		void UpdateSettingsData(Device& dev, const Data::Projection& projection);
		// Need to be called before ending every frame
		void UpdateWorldData(Device& dev, EID camera) noexcept;
		void ShowWindow(Device& dev);
	};
}