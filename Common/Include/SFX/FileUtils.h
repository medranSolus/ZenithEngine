#pragma once
#include "Ptr.h"
ZE_WARNING_PUSH
#include "FLAC/stream_decoder.h"
#include "vorbis/vorbisfile.h"
#include "opusfile.h"
ZE_WARNING_POP

namespace ZE::SFX::FileUtils
{
	// Main state of Ogg Vorbis parsing
	struct OggVorbisCtx
	{
		struct CallbackCtx
		{
			Ptr<IO::File> File;
			U64 FileEnd = 0;
			U64 ReadOffset = 0;
			OggVorbis_File Decoder = {};
			Status Code;
		};

		std::unique_ptr<CallbackCtx> IoCtx;
		Ptr<OggVorbis_File> Decoder;
		U8 ChannelCount = 0;
	};

	// Main state of Ogg Opus parsing
	struct OggOpusCtx
	{
		struct CallbackCtx
		{
			Ptr<IO::File> File;
			U64 FileEnd = 0;
			U64 ReadOffset = 0;
			Status Code;
		};

		std::unique_ptr<CallbackCtx> IoCtx;
		Ptr<OggOpusFile> Decoder;
		U8 ChannelCount = 0;
	};

	// Main state of Flac parsing
	struct FlacCtx
	{
		struct CallbackCtx
		{
			Ptr<IO::File> File;
			U64 FileEnd = 0;
			U64 RegionSize = 0;
			U64 ReadOffset = 0;
			AudioDesc Desc = {};
			Ptr<U8> OutBuffer; // Configurable output buffer
			U32 WriteOffset = 0; // After reading run inspect this member how many bytes have been written to the output buffer
			U32 BufferSize = 0; // Need to specify this after creation to control how much will be fed into the buffer at a time
			std::vector<U8> RolloverBuffer; // If decoder tried to write over buffer capacity, rest will be stored here
			Status Code;
			U8 ChannelCount = 0;
		};

		std::unique_ptr<CallbackCtx> IoCtx;
		Ptr<FLAC__StreamDecoder> Decoder;
	};

	// Initialize Ogg Vorbis parsing context
	Expected<OggVorbisCtx> GetOggVorbisCtx(IO::File& file, U64 regionEnd, AudioDesc& audioDesc) noexcept;
	// Read up to bytes of interleaved samples from Vorbis Ogg file (returns actual byte count of read samples)
	Expected<U32> ReadOggVorbis(OggVorbisCtx& ctx, U32 bytes, float* interleavedSamples) noexcept;

	// Initialize Ogg Opus parsing context
	Expected<OggOpusCtx> GetOggOpusCtx(IO::File& file, U64 regionEnd, AudioDesc& audioDesc) noexcept;
	// Read up to bytes of interleaved samples from Opus Ogg file (returns actual byte count of read samples)
	Expected<U32> ReadOggOpus(OggOpusCtx& ctx, U32 bytes, float* interleavedSamples) noexcept;

	// Initialize Flac parsing context
	Expected<FlacCtx> GetFlacCtx(IO::File& file, U64 startOffset, U64 regionSize, AudioDesc& audioDesc) noexcept;
}