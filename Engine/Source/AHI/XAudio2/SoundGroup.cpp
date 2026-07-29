#include "SFX/SoundGroup.h"

namespace ZE::AHI::XAudio2
{
	Expected<SoundGroup> SoundGroup::Create(SFX::Device& dev, U32 sampleRate, const SFX::SoundGroup* outputGroup) noexcept
	{
		SoundGroup group;
		group.processingStage = outputGroup ? outputGroup->Get().xa2.GetStage() - 1 : UINT32_MAX;
		ZE_ASSERT(group.processingStage, "Warning, too many stages used! Outputting to this group won't be possible next time!");

		XAUDIO2_SEND_DESCRIPTOR sendDest = { 0, outputGroup ? outputGroup->Get().xa2.GetVoice() : nullptr };
		XAUDIO2_VOICE_SENDS sendList = { 1, &sendDest };
		ZE_XA2_RET_FAILED_EXPECT(dev.Get().xa2.GetDevice()->CreateSubmixVoice(&group.submixVoice, 1,
			ComputeSampleRate(sampleRate), 0, group.processingStage, outputGroup ? &sendList : nullptr, nullptr));

		return group;
	}
}