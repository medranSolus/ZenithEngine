#pragma once
#include "IO/File.h"
#include "SFX/AudioBuffer.h"
#include "ChunkHeaders.h"
#include "FileResult.h"

namespace ZE::IO::WAV
{
	// Identifier of WAV file 'RIFF'
	inline constexpr U32 RIFF_MAGIC_NUMBER = ZE_MAKE_FOURCC('R', 'I', 'F', 'F');
	// Identifier of WAV file format 'WAVE'
	inline constexpr U32 WAVE_MAGIC_NUMBER = ZE_MAKE_FOURCC('W', 'A', 'V', 'E');
	// Identifier of WAV file format chunk 'fmt '
	inline constexpr U32 FORMAT_MAGIC_NUMBER = ZE_MAKE_FOURCC('f', 'm', 't', ' ');
	// Identifier of WAV file chunk 'data'
	inline constexpr U32 DATA_MAGIC_NUMBER = ZE_MAKE_FOURCC('d', 'a', 't', 'a');

	// Load and parse WAV file information from disk. Returns file handle pointing at the start of the sample data
	Expected<SFX::AudioDesc> ParseFileInfo(File& file, U64& dataStartOffset, U64 currentOffset = 0) noexcept;
	// Load sample data, must first call ParseFileInfo to get the buffer size and then allocate memory for it
	Status LoadSampleData(File& file, U64 dataStartOffset, U8* sampleBuffer, U32 blockSize, U32 writeOffset) noexcept;
}