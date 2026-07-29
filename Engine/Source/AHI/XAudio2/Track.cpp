#include "AHI/XAudio2/Track.h"

namespace ZE::AHI::XAudio2
{
	Expected<Track> Track::Create(SFX::Device& dev, const SFX::SoundGroup* group) noexcept
	{
		Track track;

		WAVEFORMATEX format = {};

		XAUDIO2_SEND_DESCRIPTOR sendDest = { 0, group ? group->Get().xa2.GetVoice() : nullptr };
		XAUDIO2_VOICE_SENDS sendList = { 1, &sendDest };
		ZE_XA2_RET_FAILED_EXPECT(dev.Get().xa2.GetDevice()->CreateSourceVoice(&track.sourceVoice, &format, 0,
			XAUDIO2_DEFAULT_FREQ_RATIO, nullptr, group ? &sendList : nullptr, nullptr));

		return track;
	}
}