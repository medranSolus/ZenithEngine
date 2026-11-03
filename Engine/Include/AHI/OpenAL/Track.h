#pragma once
#include "SFX/AudioBuffer.h"
#include "SFX/SoundGroup.h"

namespace ZE::AHI::OpenAL
{
	class Track final
	{
		ALuint source = UINT_MAX;

	public:
		Track() = default;
		ZE_CLASS_MOVE(Track);
		~Track();

		static Expected<Track> Create(SFX::Device& dev, const SFX::AudioBuffer& data, const SFX::SoundGroup* group) noexcept;

		void SetVolume(float decibels) noexcept {  }
		void SetPitch(float semitones) noexcept {  }

		void Resume() noexcept {  }
		void Pause() noexcept {  }

		Status Play(U32 loopCount) noexcept;
		Status Stop() noexcept;

		// Audio API Internal

	};
}