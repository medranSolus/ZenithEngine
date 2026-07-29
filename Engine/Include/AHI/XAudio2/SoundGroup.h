#pragma once
#include "SFX/Device.h"

namespace ZE::SFX
{
	class SoundGroup;
}
namespace ZE::AHI::XAudio2
{
	class SoundGroup final
	{
		Ptr<IXAudio2SubmixVoice> submixVoice;
		U32 processingStage = 0;

	public:
		SoundGroup() = default;
		ZE_CLASS_MOVE(SoundGroup);
		~SoundGroup() = default;

		static Expected<SoundGroup> Create(SFX::Device& dev, U32 sampleRate, const SFX::SoundGroup* outputGroup) noexcept;

		void SetVolume(float decibels) noexcept { submixVoice->SetVolume(GetVolumeLevel(decibels), XAUDIO2_COMMIT_NOW); }

		// Audio API Internal

		constexpr IXAudio2SubmixVoice* GetVoice() const noexcept { return submixVoice; }
		constexpr U32 GetStage() const noexcept { return processingStage; }
	};
}