#pragma once
#include "AudioBuffer.h"

namespace ZE::SFX
{
	// Current state of the streamed audio file depending on file source type
	struct StreamState
	{
		static constexpr U8 BUFFER_COUNT = 4;
		static constexpr U64 MAX_BUFFER_SIZE = 32 * Math::KILOBYTE;

		// Usable part of the MAX_BUFFER_SIZE that ring buffer should be allocated with
		U32 AudioBlockSize = 0;
		FileSourceType SourceType = FileSourceType::Unknown;
		union
		{
			struct
			{
				U64 DataStart;
				U64 CurrentPosition;
			} WAV;
			struct
			{
				FLAC__StreamDecoder* Decoder;
			} Flac;
		};
	};

	// Get audio file description from any file region on disk for async streaming
	Expected<StreamState> LoadStreamFileInfo(IO::File& file, U64 startOffset, FileSourceType type, AudioDesc& audioInfo) noexcept;
	// Asynchronously load next chunk in stream to the memory (can return EOF if looping is not enabled)
	Task<Status> ReadNextChunk(StreamState& state, IO::File& file, U32 readSize, U8* destBuffer, bool loopAudio) noexcept;
	// Move current stream position to the begining of the track
	Status RewindToBegining(StreamState& state) noexcept;
}