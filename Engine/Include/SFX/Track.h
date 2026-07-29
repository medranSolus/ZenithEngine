#pragma once
#if _ZE_AHI_OPENAL
#	include "AHI/OpenAL/Track.h"
#endif
#if _ZE_AHI_XAUDIO2
#	include "AHI/XAudio2/Track.h"
#endif
#include "AHI/Backend.h"

namespace ZE::SFX
{
	// Single source track with long streaming sound
	class Track final
	{
		ZE_AHI_BACKEND(Track);

	public:
		Track() = default;
		ZE_CLASS_MOVE(Track);
		~Track() = default;

		static Expected<Track> Create(Device& dev, const SoundGroup* group = nullptr) noexcept { ZE_AHI_BACKEND_CREATE(Track, dev, group); }
		ZE_AHI_BACKEND_GET(Track);

		// Main Audio API

		constexpr void SetVolume(float decibels) noexcept { ZE_AHI_BACKEND_CALL(SetVolume, decibels); }
		constexpr void SetPitch(float semitones) noexcept { ZE_AHI_BACKEND_CALL(SetPitch, semitones); }
	};
}