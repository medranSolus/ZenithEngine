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
	// Single source track with short sound
	class Track final
	{
		ZE_AHI_BACKEND(Track);

	public:
		Track() = default;
		ZE_CLASS_MOVE(Track);
		~Track() = default;

		static Expected<Track> Create(Device& dev, const SFX::AudioBuffer& data, const SoundGroup* group = nullptr) noexcept { ZE_AHI_BACKEND_CREATE(Track, dev, data, group); }
		ZE_AHI_BACKEND_GET(Track);

		// Main Audio API

		constexpr void SetVolume(float decibels) noexcept { ZE_AHI_BACKEND_CALL(SetVolume, decibels); }
		constexpr void SetPitch(float semitones) noexcept { ZE_AHI_BACKEND_CALL(SetPitch, semitones); }

		constexpr void Resume() noexcept { ZE_AHI_BACKEND_CALL(Resume); }
		constexpr void Pause() noexcept { ZE_AHI_BACKEND_CALL(Pause); }

		// 0 for no loop, UINT32_MAX for infinite loop
		Status Play(U32 loopCount = 0) noexcept { ZE_AHI_BACKEND_CALL_RET(Play, loopCount); }
		Status Stop() noexcept { ZE_AHI_BACKEND_CALL_RET(Stop); }
	};
}