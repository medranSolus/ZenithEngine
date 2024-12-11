#include "Settings.h"

namespace ZE
{
	void Settings::Destroy() noexcept
	{
		ZE_ASSERT_INIT(Initialized());
		threadPool.Stop();

		backbufferCount = 0;
		flags = 0;
		applicationName = nullptr;
		applicationVersion = 0;
		Upscaler = GFX::UpscalerType::None;
		AmbientOcclusionType = GFX::AOType::None;
	}
}