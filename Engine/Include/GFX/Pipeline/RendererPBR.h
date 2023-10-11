#pragma once
#include "DataPBR.h"
#include "ParamsPBR.h"
#include "RenderGraph.h"

namespace ZE::GFX::Pipeline
{
	// Physically Based Renderer
	class RendererPBR final : public RenderGraph
	{
		DataPBR settingsData;
		CameraPBR dynamicData;
		float blurSigma;
		XeGTAO::GTAOSettings xegtaoSettings = {};
		Float4x4 currentProjection;
		Float4 cameraRotation;

		static void SetupRenderSlots(RendererBuildData& buildData) noexcept;

		constexpr void SetupBlurKernel() noexcept;
		constexpr void SetupBlurIntensity() noexcept;
		constexpr void SetupBlurData(U32 width, U32 height) noexcept;
		constexpr void SetupXeGTAOQuality() noexcept;
		constexpr void SetupXeGTAOData(U32 width, U32 height) noexcept;

	public:
		RendererPBR() noexcept : RenderGraph(this, &settingsData, &dynamicData, sizeof(CameraPBR)) {}
		ZE_CLASS_DELETE(RendererPBR);
		virtual ~RendererPBR() = default;

		constexpr U32 GetFrameWidth() const noexcept { return Utils::SafeCast<U32>(settingsData.XeGTAOData.ViewportSize.x); }
		constexpr U32 GetFrameHeight() const noexcept { return Utils::SafeCast<U32>(settingsData.XeGTAOData.ViewportSize.y); }
		constexpr float GetFrameRation() const noexcept { return Utils::SafeCast<float>(GetFrameWidth()) / Utils::SafeCast<float>(GetFrameHeight()); }
		constexpr const Float4x4& GetProjection() const noexcept { return currentProjection; }
		constexpr const Float4& GetCameraRotation() const noexcept { return cameraRotation; }

		void Init(Device& dev, CommandList& mainList, U32 width, U32 height, const ParamsPBR& params);

		// Need to be called when data in parameters changed (also after creation of renderer)
		void UpdateSettingsData(Device& dev, const Matrix& projection);
		// Need to be called before ending every frame
		void UpdateWorldData(Device& dev, EID camera) noexcept;
		void ShowWindow(Device& dev);
	};
}