#pragma once
#include "ChannelMask.h"
#include <memory>

namespace ZE::SFX
{
	// Loaded sound data
	struct AudioBuffer
	{
		U32 Bytes = 0;
		U32 SampleRate = 0;
		ChannelMask Channels = 0;
		U8 BitsPerSample = 0;
		bool IsFloat = false;
		std::shared_ptr<U8[]> Samples;
	};

	// Load and parse audio file from disk
	Expected<AudioBuffer> LoadFile(std::string_view filename) noexcept;
}