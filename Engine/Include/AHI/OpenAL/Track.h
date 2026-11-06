#pragma once
#include "SFX/AudioBuffer.h"
#include "SFX/SoundGroup.h"

namespace ZE::AHI::OpenAL
{
	class Track final
	{
		Move<ALuint, UINT_MAX> buffer;
		Move<ALuint, UINT_MAX> source;
		std::shared_ptr<U8[]> audioData;

	public:
		Track() = default;
		ZE_CLASS_MOVE(Track);
		~Track();

		static Expected<Track> Create(SFX::Device& dev, const SFX::AudioBuffer& data, const SFX::SoundGroup* group) noexcept;

		Status SetVolume(float decibels) noexcept { alSourcef(source, AL_GAIN, GetVolumeLevel(decibels));  ZE_AL_RET_FAILED(); return {}; }
		Status SetPitch(float semitones) noexcept { alSourcef(source, AL_PITCH, semitones); ZE_AL_RET_FAILED(); return {}; }

		Status Pause() noexcept { alSourcePause(source); ZE_AL_RET_FAILED(); return {}; }
		Status Resume() noexcept;

		Status Play(U32 loopCount) noexcept;
		Status Stop() noexcept;
	};
}