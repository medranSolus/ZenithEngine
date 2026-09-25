#pragma once
#include "ChannelMask.h"
#include <memory>

namespace ZE::SFX
{
	// Types of files supported by loader
	enum class FileSourceType : U8
	{
		Unknown = 0,
		WAV, Flac, Ogg, Opus
	};

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
	Expected<AudioBuffer> LoadFile(IO::File& file, U64 startOffset, U64 regionSize, FileSourceType type) noexcept;
}