#pragma once
#include "SFX/Device.h"

namespace ZE::SFX
{
	class SoundGroup;
}
namespace ZE::AHI::OpenAL
{
	class SoundGroup final
	{

	public:
		SoundGroup() = default;
		ZE_CLASS_MOVE(SoundGroup);
		~SoundGroup() = default;

		static Expected<SoundGroup> Create(SFX::Device& dev, U32 sampleRate, const SFX::SoundGroup* outputGroup) noexcept;

		Status SetVolume(float decibels) noexcept { return {}; }

		// Audio API Internal

	};
}