#include "AHI/XAudio2/Track.h"

namespace ZE::AHI::XAudio2
{
	Expected<Track> Track::Create(SFX::Device& dev) noexcept
	{
		Track track;
		ZE_XA2_RET_FAILED_EXPECT(dev.Get().xa2.GetDevice()->CreateSourceVoice(&track.sourceVoice, nullptr, 0));

		return track;
	}
}