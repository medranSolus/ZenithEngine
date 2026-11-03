#include "SFX/SoundGroup.h"

namespace ZE::AHI::OpenAL
{
	Expected<SoundGroup> SoundGroup::Create(SFX::Device& dev, U32 sampleRate, const SFX::SoundGroup* outputGroup) noexcept
	{
		SoundGroup group;
		return group;
	}
}