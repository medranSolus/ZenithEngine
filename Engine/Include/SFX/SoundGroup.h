#pragma once
#if _ZE_AHI_OPENAL
#	include "AHI/OpenAL/SoundGroup.h"
#endif
#if _ZE_AHI_XAUDIO2
#	include "AHI/XAudio2/SoundGroup.h"
#endif
#include "AHI/Backend.h"

namespace ZE::SFX
{
	// Single source track with long streaming sound
	class SoundGroup final
	{
		ZE_AHI_BACKEND(SoundGroup);

	public:
		SoundGroup() = default;
		ZE_CLASS_MOVE(SoundGroup);
		~SoundGroup() = default;

		static Expected<SoundGroup> Create(Device& dev, U32 sampleRate, const SoundGroup* outputGroup = nullptr) noexcept { ZE_AHI_BACKEND_CREATE(SoundGroup, dev, sampleRate, outputGroup); }
		ZE_AHI_BACKEND_GET(SoundGroup);

		// Main Audio API

		constexpr void SetVolume(float decibels) noexcept { ZE_AHI_BACKEND_CALL(SetVolume, decibels); }
	};
}