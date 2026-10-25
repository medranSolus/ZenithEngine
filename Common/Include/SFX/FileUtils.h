#pragma once
#include "Ptr.h"
ZE_WARNING_PUSH
#include "vorbis/vorbisfile.h"
#include "opusfile.h"
ZE_WARNING_POP

namespace ZE::SFX::FileUtils
{
	// Main state of Ogg Vorbis parsing
	struct OggVorbisCtx
	{
		Ptr<IO::File> File;
		U64 RegionSize = 0;
		U64 ReadOffset = 0;
		OggVorbis_File Decoder = {};
		Status Code;
		U8 ChannelCount = 0;
	};

	// Main state of Ogg Opus parsing
	struct OggOpusCtx
	{
		Ptr<IO::File> File;
		U64 RegionSize = 0;
		U64 ReadOffset = 0;
		Ptr<OggOpusFile> Decoder;
		Status Code;
		U8 ChannelCount = 0;
	};

	// Initialize Ogg Vorbis parsing context
	Expected<OggVorbisCtx> GetOggVorbisCtx(IO::File& file, U64 regionSize, AudioDesc& audioDesc) noexcept;
	// Read up to bytes of interleaved samples from Vorbis Ogg file (returns actual byte count of read samples)
	Expected<U32> ReadOggVorbis(OggVorbisCtx& ctx, U32 bytes, float* interleavedSamples) noexcept;

	// Initialize Ogg Opus parsing context
	Expected<OggOpusCtx> GetOggOpusCtx(IO::File& file, U64 regionSize, AudioDesc& audioDesc) noexcept;
	// Read up to bytes of interleaved samples from Opus Ogg file (returns actual byte count of read samples)
	Expected<U32> ReadOggOpus(OggOpusCtx& ctx, U32 bytes, float* interleavedSamples) noexcept;
}