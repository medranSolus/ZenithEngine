#pragma once
#include "IO/File.h"
#include "SFX/AudioBuffer.h"

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

	// Load and parse WAV file from disk
	Expected<SFX::AudioBuffer> ParseFile(File& file) noexcept;
}