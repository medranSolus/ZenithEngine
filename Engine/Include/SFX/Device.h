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
		ZE_CLASS_DELETE(Device);
		~Device() = default;

		// Sample rate must be multiple of 100 Hz
		constexpr void Init(U32 sampleRate = 0) { ZE_AHI_BACKEND_VAR.Init(sampleRate); }
		constexpr void SwitchApi(AudioApiType nextApi, U32 sampleRate = 0) { ZE_AHI_BACKEND_VAR.Switch(nextApi, sampleRate); }
		ZE_AHI_BACKEND_GET(Device);

		// Main Audio API

	};
}