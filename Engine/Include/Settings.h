#pragma once
#include "GFX/RayTracingTier.h"
#include "GFX/VendorGPU.h"
#include "Data/Entity.h"
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
			SSSR,
			Count,
		};

	public:
		static constexpr const char* ENGINE_UUID = "5A656E69-7468-456E-6769-6E6553616669";
		static constexpr const char* ENGINE_NAME = "ZenithEngine";
		static constexpr U32 ENGINE_VERSION = Utils::MakeVersion(_ZE_VERSION_MAJOR, _ZE_VERSION_MINOR, _ZE_VERSION_PATCH);
		static constexpr const char* ENGINE_VERSION_STR = ZE_STRINGIFY(_ZE_VERSION_MAJOR) "." ZE_STRINGIFY(_ZE_VERSION_MINOR) "." ZE_STRINGIFY(_ZE_VERSION_PATCH);

		// GPU heap for buffers only
		static constexpr U64 BUFFERS_HEAP_SIZE = 256 * Math::MEGABYTE;
		// GPU heap for textures only
		static constexpr U64 TEXTURES_HEAP_SIZE = 512 * Math::MEGABYTE;
		// CPU side heap for frequently uploaded buffers
		static constexpr U64 HOST_HEAP_SIZE = 64 * Math::MEGABYTE;
		// CPU-mappable heap for upload data
		static constexpr U64 UPLOAD_HEAP_SIZE = 64 * Math::MEGABYTE;
		// Single CPU stating buffer for preparing data before upload to GPU
		static constexpr U32 STAGING_BUFFER_SIZE = 128 * Math::MEGABYTE;

		static inline GFX::VendorGPU GpuVendor = GFX::VendorGPU::Unknown;
		static inline GFX::RayTracingTier RayTracingTier = GFX::RayTracingTier::None;
		static inline PixelFormat BackbufferFormat = PixelFormat::R8G8B8A8_UNorm;

		static inline UInt2 DisplaySize = { 0, 0 };
		static inline UInt2 RenderSize = { 0, 0 };
		static inline float MaxRenderDistance = 10000.0f;
		// Time in miliseconds elapsed since last frame
		static inline double FrameTime = 0.0;

		// When creating new entities concurrently use CreateEntity() for thread-safe method
		static inline Data::Storage Data;

	private:
		static inline const char* applicationName;
		static inline U32 applicationVersion;
		static inline GfxApiType gfxApi;
		static inline GFX::UpscalerType upscaler;
		static inline GFX::AOType aoType;

		static inline ThreadPool threadPool;
		static inline std::bitset<Flags::Count> flags = 0;
		static inline U64 frameIndex = 0;
		static inline U32 backbufferCount = 0;

		static constexpr bool Initialized() noexcept { return backbufferCount != 0; }

	public:
		ZE_CLASS_DELETE(Settings);
		~Settings() = default;

		template<typename T>
		static constexpr std::shared_mutex& GetEntityMutex() noexcept { static std::shared_mutex mutex; return mutex; }

		static constexpr const char* GetAppName() noexcept { ZE_ASSERT_INIT(Initialized()); return applicationName; }
		static constexpr U32 GetAppVersion() noexcept { ZE_ASSERT_INIT(Initialized()); return applicationVersion; }
		static constexpr GfxApiType GetGfxApi() noexcept { ZE_ASSERT_INIT(Initialized()); return gfxApi; }
		static constexpr GFX::UpscalerType GetUpscaler() noexcept { ZE_ASSERT_INIT(Initialized()); return upscaler; }
		static constexpr GFX::AOType GetAOType() noexcept { ZE_ASSERT_INIT(Initialized()); return aoType; }

		static constexpr U64 GetFrameIndex() noexcept { return frameIndex; }
		static constexpr void AdvanceFrame() noexcept { ++frameIndex; }
		static constexpr float GetDisplayRatio() noexcept { ZE_ASSERT(DisplaySize.Y != 0, "Incorrect sizes!"); return Utils::SafeCast<float>(DisplaySize.X) / Utils::SafeCast<float>(DisplaySize.Y); }

		static constexpr ThreadPool& GetThreadPool() noexcept { ZE_ASSERT_INIT(Initialized()); return threadPool; }
		static constexpr U32 GetBackbufferCount() noexcept { ZE_ASSERT_INIT(Initialized()); return backbufferCount; }
		static constexpr U32 GetCurrentBackbufferIndex() noexcept { ZE_ASSERT_INIT(Initialized()); return frameIndex % GetBackbufferCount(); }
		static constexpr U32 GetCurrentChainResourceIndex() noexcept { ZE_ASSERT_INIT(Initialized()); return frameIndex % GetChainResourceCount(); }

		static constexpr bool ComputeMotionVectors() noexcept { ZE_ASSERT_INIT(Initialized()); return IsEnabledSSSR() || upscaler != GFX::UpscalerType::None && upscaler != GFX::UpscalerType::Fsr1 && upscaler != GFX::UpscalerType::NIS; }
		static constexpr bool ApplyJitter() noexcept { ZE_ASSERT_INIT(Initialized()); return upscaler != GFX::UpscalerType::None && upscaler != GFX::UpscalerType::Fsr1 && upscaler != GFX::UpscalerType::NIS; }

		static constexpr void SetGfxTags(bool enabled) noexcept { flags[Flags::GfxTags] = enabled; }
		static constexpr bool IsEnabledGfxTags() noexcept { return flags[Flags::GfxTags]; }
		static constexpr void SetU8IndexBuffers(bool enabled) noexcept { flags[Flags::IndexBufferU8] = enabled; }
		static constexpr bool IsEnabledU8IndexBuffers() noexcept { return flags[Flags::IndexBufferU8]; }
		static constexpr bool IsEnabledPIXAttaching() noexcept { ZE_ASSERT_INIT(Initialized()); return flags[Flags::AttachPIX]; }
		static constexpr bool IsEnabledGPUValidation() noexcept { ZE_ASSERT_INIT(Initialized()); return flags[Flags::GPUValidation]; }
		static constexpr bool IsEnabledSSSR() noexcept { ZE_ASSERT_INIT(Initialized()); return flags[Flags::SSSR]; }

		static EID CreateEntity() noexcept { LockGuardRW lock(GetEntityMutex<EID>()); return Data.create(); }
		static void CreateEntities(std::vector<EID>& entities) noexcept { LockGuardRW lock(GetEntityMutex<EID>()); for (EID& e : entities) e = Data.create(); }
		static void DestroyEntity(EID entity) noexcept { LockGuardRW lock(GetEntityMutex<EID>()); Data.destroy(entity); }
		static void DestroyEntities(std::vector<EID>::iterator begin, std::vector<EID>::iterator end) noexcept { LockGuardRW lock(GetEntityMutex<EID>()); for (; begin < end; ++begin) Data.destroy(*begin); }

		static constexpr U32 GetChainResourceCount() noexcept;

		static constexpr void Init(const SettingsInitParams& params) noexcept;
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

	constexpr void Settings::Init(const SettingsInitParams& params) noexcept
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

#if !_ZE_MODE_RELEASE
		flags[Flags::AttachPIX] = params.Flags & SettingsInitFlag::AllowPIXAttach;
#endif
#if _ZE_DEBUG_GFX_API
		flags[Flags::GPUValidation] = params.Flags & SettingsInitFlag::EnableGPUValidation;
#endif
#if _ZE_GFX_MARKERS
		flags[Flags::GfxTags] = true;
#endif
		if (params.Flags & SettingsInitFlag::EnableSSSR)
		{
			if (gfxApi == GfxApiType::DX12 || gfxApi == GfxApiType::Vulkan)
				flags[Flags::SSSR] = true;
			else
				Logger::Warning("Requested SSSR while using not supported GFX API! Disabling SSSR.");
		}

		backbufferCount = params.BackbufferCount;
		applicationName = params.AppName ? params.AppName : ENGINE_NAME;
		applicationVersion = params.AppVersion;
		upscaler = params.Upscaler;
		aoType = params.AmbientOcclusion;
		threadPool.Init(params.StaticThreadsCount, params.CustomThreadPoolThreadsCount);
	}
#pragma endregion
}