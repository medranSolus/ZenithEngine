#pragma once
#include "ChannelMask.h"
#include <memory>

namespace ZE::SFX
{
	// Description of the main audio parameters
	struct AudioDesc
	{
		U32 Bytes = 0;
		U32 SampleRate = 0;
		ChannelMask Channels = 0;
		U8 BitsPerSample = 0;
		bool IsFloat = false;
	};

	// Loaded sound data
	struct AudioBuffer
	{
		AudioDesc Desc = {};
		std::shared_ptr<U8[]> Samples;
	};

	// Load and parse audio file from disk
	Expected<AudioBuffer> LoadFile(std::string_view filename) noexcept;
}