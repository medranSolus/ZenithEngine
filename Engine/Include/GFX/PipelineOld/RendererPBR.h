#pragma once

namespace ZE::GFX::Pipeline
{
	// Physically Based Renderer
	class RendererPBR final
	{
		//DataPBR settingsData = {};
		//CameraPBR dynamicData = {};
		float blurSigma = 0.0f;

		//Data::Projection currentProjectionData = {};
		Float4x4 currentProjection = {};
		Float4x4 prevViewProjectionTps = {};
		Float4 cameraRotation = {};

		float iblFactor = 0.55f;
		float sharpness = 0.5f;
		bool enableSharpening = true;
		U32 jitterIndex = 0;

	public:
		RendererPBR() = default;
		ZE_CLASS_DELETE(RendererPBR);
		virtual ~RendererPBR() = default;

		//constexpr const Data::Projection& GetProjectionData() const noexcept { return currentProjectionData; }
		constexpr const Float4x4& GetProjection() const noexcept { return currentProjection; }
		constexpr const Float4x4& GetPrevViewProjectionTps() const noexcept { return prevViewProjectionTps; }
		constexpr const Float4& GetCameraRotation() const noexcept { return cameraRotation; }
		constexpr float GetIBLFactor() const noexcept { return iblFactor; }
		constexpr float GetSharpness() const noexcept { return enableSharpening ? sharpness : 0.0f; }
		constexpr bool IsSharpeningEnabled() const noexcept { return enableSharpening; }

		//void Init(Device& dev, CommandList& mainList, Data::AssetsStreamer& assets, const ParamsPBR& params);

		// Need to be called when data in parameters changed (also after creation of renderer)
		//void UpdateSettingsData(const Data::Projection& projection) noexcept;
		//void SetInverseViewProjection(EID camera) noexcept;
		// Need to be called before ending every frame
		//void UpdateWorldData(Device& dev, EID camera) noexcept;
		//void ShowWindow(Device& dev, Data::AssetsStreamer& assets);
	};
}