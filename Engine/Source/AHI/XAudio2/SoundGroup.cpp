#include "SFX/SoundGroup.h"

namespace ZE::AHI::XAudio2
{
	Expected<SoundGroup> SoundGroup::Create(SFX::Device& dev, U32 sampleRate, const SFX::SoundGroup* outputGroup) noexcept
	{
		SoundGroup group;

		XAUDIO2_SEND_DESCRIPTOR sendDest = { 0, outputGroup ? outputGroup->Get().xa2.GetVoice() : nullptr };
		XAUDIO2_VOICE_SENDS sendList = { 1, &sendDest };
		ZE_XA2_RET_FAILED_EXPECT(dev.Get().xa2.GetDevice()->CreateSubmixVoice(&group.submixVoice, 1,
			ComputeSampleRate(sampleRate), 0, 0, outputGroup ? &sendList : nullptr, nullptr));

		return group;
	}
}