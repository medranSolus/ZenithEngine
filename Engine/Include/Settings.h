#pragma once
#include "IO/AsyncBackgroundThread.h"
#include "Data/SystemsBank.h"
#include "GFX/RayTracingTier.h"
#include "GFX/VendorGPU.h"
#include "SettingsInitParams.h"
#include "ThreadPool.h"

namespace ZE
{
	// Global engine settings
	class Settings final
	{
		enum Flags : U8
		{
			GfxTags,
			IndexBufferU8,
			AttachPIX,
			GPUValidation,
			EnabledSSSR,
			SupportedSSSR,
			AsyncAO,
			CopySourceGPUData,
			NoCulling,
			ImGui,
			SplitRenderSubmissions,
			IBL,
			DebugView,
			SupportedAsyncQueue,
			Count,
		};

	public:
		static constexpr const char* ENGINE_UUID = "5A656E69-7468-456E-6769-6E6553616669";
		static constexpr const char* ENGINE_NAME = "ZenithEngine";
		static constexpr const wchar_t* ENGINE_NAME_WIDE = L"ZenithEngine";
		static constexpr const char* ENGINE_DISPLAY_NAME = "Zenith Engine";
		static constexpr U32 ENGINE_VERSION = Utils::MakeVersion(ZE_VERSION_MAJOR, ZE_VERSION_MINOR, ZE_VERSION_PATCH);
		static constexpr const char* ENGINE_VERSION_STR = ZE_STRINGIFY_VERSION(ZE_VERSION_MAJOR, ZE_VERSION_MINOR, ZE_VERSION_PATCH);

		// Have to be adjusted per-platform
		static constexpr U8 MAX_RENDER_TARGETS = 8;

		static inline GFX::VendorGPU GpuVendor = GFX::VendorGPU::Unknown;
		static inline GFX::RayTracingTier RayTracingTier = GFX::RayTracingTier::None;
		static inline GFX::UpscalerType Upscaler = GFX::UpscalerType::None;
		static inline GFX::AOType AmbientOcclusionType = GFX::AOType::None;
		static inline GFX::TonemapperType Tonemapper = GFX::TonemapperType::Exposure;
		static inline PixelFormat BackbufferFormat = PixelFormat::B8G8R8A8_UNorm_SRGB;

		static inline UInt2 DisplaySize = { 0, 0 };
		static inline UInt2 RenderSize = { 0, 0 };
		static inline float MaxRenderDistance = 10000.0f;
		// Time in miliseconds elapsed since last frame
		static inline double FrameTime = 0.0;

		static inline Data::SystemsBank DataBank;

	private:
		static inline const char* applicationName = nullptr;
		static inline U32 applicationVersion = 0;
		static inline GfxApiType gfxApi = GfxApiType::None;
		static inline AudioApiType audioApi = AudioApiType::None;
		static inline HeapParams heapSizes = {};

		static inline ThreadPool threadPool;
		static inline IO::AsyncBackgroundThread ioThread;
		static inline std::bitset<Flags::Count> flags = 0;
		static inline U64 frameIndex = UINT64_MAX; // Sets to 0 when engine is starting up
		static inline U32 backbufferCount = 0;

		static constexpr bool Initialized() noexcept { return backbufferCount != 0; }

	public:
		ZE_CLASS_DELETE(Settings);
		~Settings() = default;

#if !_ZE_MODE_RELEASE
		// Utility debug mutex to aid in debugging multi-threaded issues, should not be used in production code
		static constexpr std::shared_mutex& GetDebugMutex() noexcept { static std::shared_mutex mutex; return mutex; }
#endif
		static constexpr const char* GetAppName() noexcept { ZE_ASSERT_INIT(Initialized()); return applicationName; }
		static constexpr U32 GetAppVersion() noexcept { ZE_ASSERT_INIT(Initialized()); return applicationVersion; }
		static constexpr GfxApiType GetGfxApi() noexcept { ZE_ASSERT_INIT(Initialized() || gfxApi == GfxApiType::None); return gfxApi; }
		static constexpr AudioApiType GetAudioApi() noexcept { ZE_ASSERT_INIT(Initialized() || audioApi == AudioApiType::None); return audioApi; }
		static constexpr const HeapParams& GetHeapSizes() noexcept { ZE_ASSERT_INIT(Initialized()); return heapSizes; }

		static constexpr U64 GetFrameIndex() noexcept { return frameIndex; }
		static constexpr void AdvanceFrame() noexcept { ++frameIndex; }
		static constexpr float GetDisplayRatio() noexcept { ZE_ASSERT(DisplaySize.Y != 0, "Incorrect sizes!"); return Utils::SafeCast<float>(DisplaySize.X) / Utils::SafeCast<float>(DisplaySize.Y); }

		static constexpr ThreadPool& GetThreadPool() noexcept { ZE_ASSERT_INIT(Initialized()); return threadPool; }
		static constexpr U32 GetBackbufferCount() noexcept { ZE_ASSERT_INIT(Initialized()); return backbufferCount; }
		static constexpr U32 GetCurrentBackbufferIndex() noexcept { ZE_ASSERT_INIT(Initialized()); return frameIndex % GetBackbufferCount(); }
		static constexpr U32 GetCurrentChainResourceIndex() noexcept { ZE_ASSERT_INIT(Initialized()); return frameIndex % GetChainResourceCount(); }

		static constexpr bool ComputeMotionVectors() noexcept { ZE_ASSERT_INIT(Initialized()); return IsEnabledSSSR() || IsMotionRequired(Upscaler); }
		static constexpr bool ApplyJitter() noexcept { ZE_ASSERT_INIT(Initialized()); return IsJitterRequired(Upscaler); }

		static constexpr bool IsEnabledGfxTags() noexcept { return flags[Flags::GfxTags]; }
		static constexpr bool IsEnabledU8IndexBuffers() noexcept { return flags[Flags::IndexBufferU8]; }
		static constexpr bool IsEnabledPIXAttaching() noexcept { ZE_ASSERT_INIT(Initialized()); return flags[Flags::AttachPIX]; }
		static constexpr bool IsEnabledGPUValidation() noexcept { ZE_ASSERT_INIT(Initialized()); return flags[Flags::GPUValidation]; }
		static constexpr bool IsEnabledSSSR() noexcept { ZE_ASSERT_INIT(Initialized()); return flags[Flags::EnabledSSSR] && IsSupportedSSSR(); }
		static constexpr bool IsSupportedSSSR() noexcept { ZE_ASSERT_INIT(Initialized()); return flags[Flags::SupportedSSSR]; }
		static constexpr bool IsEnabledAsyncAO() noexcept { ZE_ASSERT_INIT(Initialized()); return flags[Flags::AsyncAO] && IsSupportedAsyncQueue(); }
		static constexpr bool IsEnabledCopySourceGPUData() noexcept { ZE_ASSERT_INIT(Initialized()); return flags[Flags::CopySourceGPUData]; }
		static constexpr bool IsEnabledNoCulling() noexcept { ZE_ASSERT_INIT(Initialized()); return flags[Flags::NoCulling]; }
		static constexpr bool IsEnabledImGui() noexcept { return flags[Flags::ImGui]; }
		static constexpr bool IsEnabledSplitRenderSubmissions() noexcept { return flags[Flags::SplitRenderSubmissions]; }
		static constexpr bool IsEnabledIBL() noexcept { return flags[Flags::IBL]; }
		static constexpr bool IsEnabledDebugView() noexcept { return flags[Flags::DebugView]; }
		static constexpr bool IsSupportedAsyncQueue() noexcept { return flags[Flags::SupportedAsyncQueue]; }

		static constexpr void SetGfxTags(bool enabled) noexcept { flags[Flags::GfxTags] = enabled; }
		static constexpr void SetU8IndexBuffers(bool enabled) noexcept { flags[Flags::IndexBufferU8] = enabled; }
		static constexpr void SetImGui(bool enabled) noexcept { flags[Flags::ImGui] = enabled; }
		static constexpr void SetSSSR(bool enabled) noexcept { flags[Flags::EnabledSSSR] = enabled; }
		static constexpr void SetGfxSupportSSSR(bool enabled) noexcept { flags[Flags::SupportedSSSR] = enabled; }
		static constexpr void SetIBL(bool enabled) noexcept { flags[Flags::IBL] = enabled; }
		static constexpr void SetDebugView(bool enabled) noexcept { flags[Flags::DebugView] = enabled; }
		static constexpr void SetGfxSupportAsyncQueue(bool enabled) noexcept { flags[Flags::SupportedAsyncQueue] = enabled; }

		static constexpr U32 GetChainResourceCount() noexcept;

		static Status Init(const SettingsInitParams& params) noexcept;
		static void Destroy() noexcept;
	};

#pragma region Functions
	constexpr U32 Settings::GetChainResourceCount() noexcept
	{
		ZE_ASSERT_INIT(Initialized());

		switch (GetGfxApi())
		{
		default:
			ZE_ENUM_UNHANDLED();
		case GfxApiType::DX11:
		case GfxApiType::OpenGL:
			return 1;
		case GfxApiType::DX12:
		case GfxApiType::Vulkan:
			return GetBackbufferCount();
		}
	}
#pragma endregion
}