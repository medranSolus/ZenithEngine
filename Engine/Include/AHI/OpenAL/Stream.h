#pragma once
#include "SFX/StreamDesc.h"

namespace ZE::AHI::OpenAL
{
	class Stream final
	{

		Move<ALuint, UINT_MAX> source;

	public:
		Stream() = default;
		ZE_CLASS_MOVE(Stream);
		~Stream();

		static Expected<Stream> Create(SFX::Device& dev, const SFX::StreamDesc& desc) noexcept;

		Status SetVolume(float decibels) noexcept { alSourcef(source, AL_GAIN, GetGainLevel(decibels));  ZE_AL_RET_FAILED(); return {}; }
		Status SetPitch(float semitones) noexcept { alSourcef(source, AL_PITCH, GetPitchLevel(semitones)); ZE_AL_RET_FAILED(); return {}; }

		Status Resume() noexcept { return ResumePlayback(source); }
		Status Pause() noexcept { alSourcePause(source); ZE_AL_RET_FAILED(); return {}; }

		bool IsLooping() const noexcept {  }

		Task<Status> ResetPosition(bool autoPlay, bool enableLoop) noexcept;
	};
}