#pragma once

namespace ZE::GFX::Pipeline
{
	// Physically Based Renderer
	class RendererPBR final
	{
		//DataPBR settingsData = {};
		//CameraPBR dynamicData = {};
		float blurSigma = 0.0f;

		float iblFactor = 0.55f;
		float sharpness = 0.5f;
		bool enableSharpening = true;

	public:
		RendererPBR() = default;
		ZE_CLASS_DELETE(RendererPBR);
		virtual ~RendererPBR() = default;

		constexpr float GetIBLFactor() const noexcept { return iblFactor; }
		constexpr float GetSharpness() const noexcept { return enableSharpening ? sharpness : 0.0f; }
		constexpr bool IsSharpeningEnabled() const noexcept { return enableSharpening; }


		void ShowWindow();
	};
}