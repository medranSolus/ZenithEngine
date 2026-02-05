#include "Settings.h"

namespace ZE
{
	void Settings::Destroy() noexcept
	{
		ZE_ASSERT_INIT(Initialized());
		threadPool.Stop();

		GpuVendor = GFX::VendorGPU::Unknown;
		RayTracingTier = GFX::RayTracingTier::None;
		Upscaler = GFX::UpscalerType::None;
		AmbientOcclusionType = GFX::AOType::None;
		Tonemapper = GFX::TonemapperType::Exposure;
		DisplaySize = { 0, 0 };
		RenderSize = { 0, 0 };
		MaxRenderDistance = 10000.0f;
		FrameTime = 0.0;

		applicationName = nullptr;
		applicationVersion = 0;
		gfxApi = GfxApiType::None;
		audioApi = AudioApiType::None;
		flags = 0;
		frameIndex = UINT64_MAX;
		backbufferCount = 0;
	}
}