#pragma once
#if _ZE_AHI_OPENAL
#	include "AHI/OpenAL/Device.h"
#endif
#if _ZE_AHI_XAUDIO2
#	include "AHI/XAudio2/Device.h"
#endif
#include "AHI/Backend.h"

namespace ZE::SFX
{
	// Main Sound Card device
	class Device final
	{
		ZE_AHI_BACKEND(Device);

	public:
		Device() = default;
		ZE_CLASS_MOVE(Device);
		~Device() = default;

		// Sample rate must be multiple of 100 Hz
		static Expected<Device> Create(U32 sampleRate = 0) noexcept { ZE_AHI_BACKEND_CREATE(Device, sampleRate); }
		ZE_AHI_BACKEND_GET(Device);

		// Main Audio API

		constexpr void SetVolume(float decibels) noexcept { ZE_AHI_BACKEND_CALL(SetVolume, decibels); }
	};
}