#include "SFX/AudioBuffer.h"
#include "SFX/FileError.h"
#include "SFX/FileUtils.h"
#include "IO/WAV/Utils.h"

namespace ZE::SFX
{
	Expected<AudioBuffer> LoadFile(IO::File& file, U64 startOffset, U64 regionSize, FileSourceType type) noexcept
	{
		U32 writeOffset = 0;
		AudioBuffer buffer = {};
		switch (type)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case FileSourceType::Unknown:
		{
			ZE_FAIL("Unknown audio file format!");
			return std::unexpected(ZE_WAV_ERROR(IO::WAV::FileResult::Unknown));
		}
		case FileSourceType::WAV:
		{
			U64 dataStart = 0;
			ZE_EXPECT_RET_FAILED(buffer.Desc, IO::WAV::ParseFileInfo(file, dataStart, startOffset));
			buffer.Samples = std::make_shared_for_overwrite<U8[]>(buffer.Desc.Bytes);
			
			Status stat = file.Read(buffer.Samples.get(), buffer.Desc.Bytes, dataStart);
			if (stat)
			{
				if (IO::EofResult::IsEOF(stat))
				{
					writeOffset = IO::EofResult::GetRealBytes(stat);
					ZE_WARNING("While reading WAV file expected " + std::to_string(buffer.Desc.Bytes) + " bytes but got only " + std::to_string(writeOffset));
				}
				else
					return std::unexpected(stat);
			}
			else
				writeOffset = buffer.Desc.Bytes;
			break;
		}
		case FileSourceType::Flac:
		{
			FileUtils::FlacCtx ctx = {};
			ZE_EXPECT_RET_FAILED(ctx, FileUtils::GetFlacCtx(file, startOffset, regionSize, buffer.Desc));
			buffer.Samples = std::make_shared_for_overwrite<U8[]>(buffer.Desc.Bytes);
			ctx.IoCtx->OutBuffer = buffer.Samples.get();
			ctx.IoCtx->BufferSize = buffer.Desc.Bytes;

			FLAC__stream_decoder_process_until_end_of_stream(ctx.Decoder);
			FLAC__stream_decoder_finish(ctx.Decoder);
			FLAC__stream_decoder_delete(ctx.Decoder);

			if (ctx.IoCtx->Code)
				return std::unexpected(ctx.IoCtx->Code);
			writeOffset = ctx.IoCtx->WriteOffset;
			break;
		}
		case FileSourceType::OggVorbis:
		{
			FileUtils::OggVorbisCtx ctx = {};
			ZE_EXPECT_RET_FAILED(ctx, FileUtils::GetOggVorbisCtx(file, startOffset + regionSize, buffer.Desc));
			buffer.Samples = std::make_shared_for_overwrite<U8[]>(buffer.Desc.Bytes);

			auto expWriteBytes = FileUtils::ReadOggVorbis(ctx, buffer.Desc.Bytes, reinterpret_cast<float*>(buffer.Samples.get()));
			if (expWriteBytes)
				writeOffset = *expWriteBytes;
			else
				ctx.IoCtx->Code = expWriteBytes.error();
			ov_clear(ctx.Decoder);

			if (ctx.IoCtx->Code)
				return std::unexpected(ctx.IoCtx->Code);
			break;
		}
		case FileSourceType::OggOpus:
		{
			FileUtils::OggOpusCtx ctx = {};
			ZE_EXPECT_RET_FAILED(ctx, FileUtils::GetOggOpusCtx(file, startOffset + regionSize, buffer.Desc));
			buffer.Samples = std::make_shared_for_overwrite<U8[]>(buffer.Desc.Bytes);

			auto expWriteBytes = FileUtils::ReadOggOpus(ctx, buffer.Desc.Bytes, reinterpret_cast<float*>(buffer.Samples.get()));
			if (expWriteBytes)
				writeOffset = *expWriteBytes;
			else
				ctx.IoCtx->Code = expWriteBytes.error();
			op_free(ctx.Decoder);

			if (ctx.IoCtx->Code)
				return std::unexpected(ctx.IoCtx->Code);
			break;
		}
		}

		// Re-fit buffer to proper size
		if (writeOffset < buffer.Desc.Bytes)
		{
			auto audioBuffer = std::make_shared_for_overwrite<U8[]>(writeOffset);
			std::memcpy(audioBuffer.get(), buffer.Samples.get(), writeOffset);
			buffer.Samples = std::move(audioBuffer);
			buffer.Desc.Bytes = writeOffset;
		}
		return buffer;
	}
}