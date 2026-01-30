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

		constexpr void Init() { ZE_AHI_BACKEND_VAR.Init(); }
		constexpr void SwitchApi(AudioApiType nextApi) { ZE_AHI_BACKEND_VAR.Switch(nextApi); }
		ZE_AHI_BACKEND_GET(Device);

		// Main Audio API

	};
}