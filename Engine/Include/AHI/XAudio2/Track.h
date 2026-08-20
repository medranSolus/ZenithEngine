#pragma once
#include "SFX/AudioBuffer.h"
#include "SFX/SoundGroup.h"

namespace ZE::AHI::XAudio2
{
	class Track final
	{
		Ptr<IXAudio2SourceVoice> sourceVoice;
		std::shared_ptr<U8[]> audioData;
		U32 dataSize = 0;

	public:
		Track() = default;
		ZE_CLASS_MOVE(Track);
		~Track();

		static Expected<Track> Create(SFX::Device& dev, const SFX::AudioBuffer& data, const SFX::SoundGroup* group) noexcept;

		void SetVolume(float decibels) noexcept { sourceVoice->SetVolume(GetVolumeLevel(decibels), XAUDIO2_COMMIT_NOW); }
		void SetPitch(float semitones) noexcept { sourceVoice->SetFrequencyRatio(GetPitchLevel(semitones), XAUDIO2_COMMIT_NOW); }

		void Resume() noexcept { sourceVoice->Start(0, XAUDIO2_COMMIT_NOW); }
		void Pause() noexcept { sourceVoice->Stop(0, XAUDIO2_COMMIT_NOW); }

		Status Play(U32 loopCount) noexcept;
		Status Stop() noexcept;

		// Audio API Internal

	};
}