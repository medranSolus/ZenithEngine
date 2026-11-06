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

		Status SetVolume(float decibels) noexcept { return ZE_XA2_ERROR(sourceVoice->SetVolume(GetVolumeLevel(decibels), XAUDIO2_COMMIT_NOW)); }
		Status SetPitch(float semitones) noexcept { return ZE_XA2_ERROR(sourceVoice->SetFrequencyRatio(GetPitchLevel(semitones), XAUDIO2_COMMIT_NOW)); }

		Status Resume() noexcept { return ZE_XA2_ERROR(sourceVoice->Start(0, XAUDIO2_COMMIT_NOW)); }
		Status Pause() noexcept { return ZE_XA2_ERROR(sourceVoice->Stop(0, XAUDIO2_COMMIT_NOW)); }

		Status Play(U32 loopCount) noexcept;
		Status Stop() noexcept;
	};
}