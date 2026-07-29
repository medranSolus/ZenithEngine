#pragma once
#include "SFX/SoundGroup.h"

namespace ZE::AHI::XAudio2
{
	class Track final
	{
		Ptr<IXAudio2SourceVoice> sourceVoice;

	public:
		Track() = default;
		ZE_CLASS_MOVE(Track);
		~Track() = default;

		static Expected<Track> Create(SFX::Device& dev, const SFX::SoundGroup* group) noexcept;

		void SetVolume(float decibels) noexcept { sourceVoice->SetVolume(GetVolumeLevel(decibels), XAUDIO2_COMMIT_NOW); }
		void SetPitch(float semitones) noexcept { sourceVoice->SetFrequencyRatio(GetPitchLevel(semitones), XAUDIO2_COMMIT_NOW); }

		// Audio API Internal

	};
}