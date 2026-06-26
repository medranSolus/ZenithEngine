#pragma once
#include "SFX/Device.h"

namespace ZE::AHI::XAudio2
{
	class Track final
	{
		Ptr<IXAudio2SourceVoice> sourceVoice;

	public:
		Track() = default;
		ZE_CLASS_MOVE(Track);
		~Track() = default;

		static Expected<Track> Create(SFX::Device& dev) noexcept;

		// Audio API Internal

	};
}