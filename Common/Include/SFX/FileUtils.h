#pragma once
ZE_WARNING_PUSH
#include "vorbis/vorbisfile.h"
#include "opusfile.h"
ZE_WARNING_POP

namespace ZE::SFX::FileUtils
{
	// Parse main info from Vorbis encoded stream in Ogg container
	Expected<AudioDesc> GetOggVorbisInfo(OggVorbis_File& decoder) noexcept;
	// Read up to bytes of interleaved samples from Vorbis Ogg file (returns actual byte count of read samples)
	Expected<U32> ReadOggVorbis(OggVorbis_File& decoder, U32 bytes, ChannelMask mask, float* interleavedSamples) noexcept;

	// Parse main info from Opus encoded stream in Ogg container
	Expected<AudioDesc> GetOggOpusInfo(OggOpusFile* decoder) noexcept;
	// Read up to bytes of interleaved samples from Opus Ogg file (returns actual byte count of read samples)
	Expected<U32> ReadOggOpus(OggOpusFile* decoder, U32 bytes, ChannelMask mask, float* interleavedSamples) noexcept;
}