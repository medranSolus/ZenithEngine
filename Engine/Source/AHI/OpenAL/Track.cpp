#include "AHI/OpenAL/Track.h"

namespace ZE::AHI::OpenAL
{
	Track::~Track()
	{
		if (source != UINT32_MAX)
		{

		}
	}

	Expected<Track> Track::Create(SFX::Device& dev, const SFX::AudioBuffer& data, const SFX::SoundGroup* group) noexcept
	{
		Track track;
		return track;
	}

	Status Track::Play(U32 loopCount) noexcept
	{
		Resume();
		return {};
	}

	Status Track::Stop() noexcept
	{
		Pause();

		return {};
	}
}