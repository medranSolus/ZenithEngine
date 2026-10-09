#include "Settings.h"

namespace ZE
{
	Status Settings::Init(const SettingsInitParams& params) noexcept
	{
		ZE_ASSERT(!Initialized(), "Already initialized!");
		ZE_ASSERT(params.BackbufferCount > 1 && params.BackbufferCount < 17, "Incorrect params!");

		gfxApi = params.GraphicsAPI;
		ZE_ASSERT(gfxApi == GfxApiType::DX11 && _ZE_RHI_DX11 || gfxApi != GfxApiType::DX11,
			"DirectX 11 API is not enabled in current build!");
		ZE_ASSERT(gfxApi == GfxApiType::DX12 && _ZE_RHI_DX12 || gfxApi != GfxApiType::DX12,
			"DirectX 12 API is not enabled in current build!");
		ZE_ASSERT(gfxApi == GfxApiType::OpenGL && _ZE_RHI_GL || gfxApi != GfxApiType::OpenGL,
			"OpenGL API is not enabled in current build!");
		ZE_ASSERT(gfxApi == GfxApiType::Vulkan && _ZE_RHI_VK || gfxApi != GfxApiType::Vulkan,
			"Vulkan API is not enabled in current build!");

		audioApi = params.AudioAPI;
		ZE_ASSERT(audioApi == AudioApiType::XAudio2 && _ZE_AHI_XAUDIO2 || audioApi != AudioApiType::XAudio2,
			"XAudio2 API is not enabled in current build!");
		ZE_ASSERT(audioApi == AudioApiType::OpenAL && _ZE_AHI_OPENAL || audioApi != AudioApiType::OpenAL,
			"OpenAL API is not enabled in current build!");

		heapSizes = params.HeapSizes;

#if !_ZE_MODE_RELEASE
		flags[Flags::AttachPIX] = params.Flags & SettingsInitFlag::AllowPIXAttach;
		flags[Flags::CopySourceGPUData] = params.Flags & SettingsInitFlag::AlwaysCopySourceGPUData;
		flags[Flags::NoCulling] = params.Flags & SettingsInitFlag::DisableCulling;
		flags[Flags::ImGui] = true;
		flags[Flags::SplitRenderSubmissions] = params.Flags & SettingsInitFlag::SplitRenderSubmissions;
#endif
#if _ZE_DEBUG_GFX_API
		flags[Flags::GPUValidation] = params.Flags & SettingsInitFlag::EnableGPUValidation;
#endif
#if _ZE_GFX_MARKERS
		flags[Flags::GfxTags] = true;
#endif
		flags[Flags::EnabledSSSR] = params.Flags & SettingsInitFlag::EnableSSSR;
		flags[Flags::AsyncAO] = params.Flags & SettingsInitFlag::AsyncAO;
		flags[Flags::IBL] = params.Flags & SettingsInitFlag::EnableIBL;

		backbufferCount = params.BackbufferCount;
		applicationName = params.AppName ? params.AppName : ENGINE_NAME;
		applicationVersion = params.AppVersion;
		Upscaler = params.Upscaler;
		AmbientOcclusionType = params.AmbientOcclusion;
		Tonemapper = params.Tonemapper;
		threadPool.Init(params.StaticThreadsCount, params.CustomThreadPoolThreadsCount);

		JobSteal::RegisterThreadPool(&threadPool);
		return ioThread.Start(threadPool, threadPool.GetWorkerThreadsCount());
	}

	void Settings::Destroy() noexcept
	{
		ZE_ASSERT_INIT(Initialized());
		ioThread.Stop();
		JobSteal::RegisterThreadPool(nullptr);
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