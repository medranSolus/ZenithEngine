#include "SFX/StreamState.h"
#include "SFX/FileError.h"
#include "IO/WAV/Utils.h"

namespace ZE::SFX
{
	void StreamState::MoveFrom(StreamState&& state) noexcept
	{
		AudioBlockSize = state.AudioBlockSize;
		SourceType = state.SourceType;
		switch (SourceType)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case FileSourceType::Unknown:
			break;
		case FileSourceType::WAV:
		{
			WAV = std::move(state.WAV);
			break;
		}
		case FileSourceType::Flac:
		{
			Flac = std::move(state.Flac);
			break;
		}
		case FileSourceType::OggVorbis:
		{
			OggVorbis = std::move(state.OggVorbis);
			break;
		}
		case FileSourceType::OggOpus:
		{
			OggOpus = std::move(state.OggOpus);
			break;
		}
		}
	}

	Expected<StreamState> LoadStreamFileInfo(IO::File& file, U64 startOffset,
		U64 regionSize, FileSourceType type, AudioDesc& audioInfo) noexcept
	{
		StreamState state = {};
		state.SourceType = type;
		switch (type)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case FileSourceType::Unknown:
		{
			ZE_FAIL("Unknown audio file format!");
			return std::unexpected(IO::WAV::Error::Make(IO::WAV::FileResult::Unknown));
		}
		case FileSourceType::WAV:
		{
			ZE_EXPECT_RET_FAILED(audioInfo, IO::WAV::ParseFileInfo(file, state.WAV.DataStart, startOffset));
			state.WAV.File = &file;
			break;
		}
		case FileSourceType::Flac:
		{
			FLAC__StreamDecoder* decoder = FLAC__stream_decoder_new();
			if (!decoder)
			{
				ZE_FAIL("Failed to create FLAC decoder!");
				return std::unexpected(ZE_FLAC_DECODER_INIT_ERROR(FLAC__STREAM_DECODER_INIT_STATUS_MEMORY_ALLOCATION_ERROR));
			}
			FLAC__stream_decoder_set_md5_checking(decoder, _ZE_MODE_DEBUG);
			FLAC__stream_decoder_set_metadata_respond(decoder, FLAC__METADATA_TYPE_STREAMINFO);
			FLAC__stream_decoder_set_metadata_respond(decoder, FLAC__METADATA_TYPE_VORBIS_COMMENT);

			struct FlacCtx
			{
				IO::File& File;
				U64 StartOffset = 0;
				U64 EndOffset = 0;
				U64 ReadOffset = 0;
				U32 WriteOffset = 0;
				U8 ChannelCount = 0;
				U8 CurrentBuffer = 0;
				AudioDesc AudioInfo = {};
				std::array<U8*, 4> Buffers;
				Status Code;
			};

			FlacCtx ctx = { file };
			FLAC__StreamDecoderReadCallback read = [](const FLAC__StreamDecoder* decoder, FLAC__byte buffer[], size_t* bytes, void* ctx) noexcept -> FLAC__StreamDecoderReadStatus
				{
					ZE_ASSERT(ctx, "Empty FLAC context!");
					ZE_ASSERT(bytes, "Empty FLAC byte count!");
					ZE_ASSERT(buffer, "Empty FLAC output buffer!");

					if (*bytes > 0)
					{
						auto& context = *reinterpret_cast<FlacCtx*>(ctx);
						auto status = context.File.ReadAsync(buffer, Utils::SafeCast<U32>(*bytes), context.ReadOffset).Get();

						if (status)
						{
							if (IO::EofResult::IsEOF(status))
							{
								*bytes = IO::EofResult::GetRealBytes(status);
								context.ReadOffset += *bytes;
								return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
							}
							ZE_CODE_ERROR(status, "Error reading FLAC stream file!");
							return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
						}
						context.ReadOffset += *bytes;
						return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
					}
					return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
				};
			FLAC__StreamDecoderSeekCallback seek = [](const FLAC__StreamDecoder* decoder, FLAC__uint64 offset, void* ctx) noexcept -> FLAC__StreamDecoderSeekStatus
				{
					ZE_ASSERT(ctx, "Empty FLAC context!");
					reinterpret_cast<FlacCtx*>(ctx)->ReadOffset = offset;
					return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
				};
			FLAC__StreamDecoderTellCallback tell = [](const FLAC__StreamDecoder* decoder, FLAC__uint64* offset, void* ctx) noexcept -> FLAC__StreamDecoderTellStatus
				{
					ZE_ASSERT(ctx, "Empty FLAC context!");
					ZE_ASSERT(offset, "Empty FLAC offset!");

					*offset = reinterpret_cast<FlacCtx*>(ctx)->ReadOffset;
					return FLAC__STREAM_DECODER_TELL_STATUS_OK;
				};
			FLAC__StreamDecoderLengthCallback length = [](const FLAC__StreamDecoder* decoder, FLAC__uint64* streamLen, void* ctx) noexcept -> FLAC__StreamDecoderLengthStatus
				{
					ZE_ASSERT(ctx, "Empty FLAC context!");
					ZE_ASSERT(streamLen, "Empty stream length!");

					auto& context = *reinterpret_cast<FlacCtx*>(ctx);
					U64 size = context.EndOffset - context.StartOffset;
					if (size)
					{
						*streamLen = size;
						return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
					}
					return FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR;
				};
			FLAC__StreamDecoderEofCallback eof = [](const FLAC__StreamDecoder* decoder, void* ctx) noexcept -> FLAC__bool
				{
					ZE_ASSERT(ctx, "Empty FLAC context!");
					auto& context = *reinterpret_cast<FlacCtx*>(ctx);
					return context.ReadOffset >= context.EndOffset;
				};
			FLAC__StreamDecoderWriteCallback write = [](const FLAC__StreamDecoder* decoder, const FLAC__Frame* frame, const FLAC__int32* const buffer[], void* ctx) noexcept -> FLAC__StreamDecoderWriteStatus
				{
					ZE_ASSERT(frame, "Empty FLAC frame!");
					ZE_ASSERT(buffer, "Empty FLAC buffer!");
					ZE_ASSERT(ctx, "Empty FLAC context!");

					auto& context = *reinterpret_cast<FlacCtx*>(ctx);



					// TODO: HERE COMES MAGIC OF WAITING ETC





					// Sanity check
					const U8 sampleBytes = Math::DivideRoundUp<U8>(context.AudioInfo.BitsPerSample, 8);
					if (context.AudioInfo.Bytes < context.WriteOffset + (frame->header.blocksize * context.ChannelCount * sampleBytes))
						return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;

					// Copy into interleaved format
					for (U32 i = 0; i < frame->header.blocksize; ++i)
					{
						for (U8 j = 0; j < context.ChannelCount; ++j)
						{
							//std::memcpy(context.Buffer.Samples.get() + context.WriteOffset, buffer[j] + i, sampleBytes);
							context.WriteOffset += sampleBytes;
						}
					}
					return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
				};
			FLAC__StreamDecoderMetadataCallback metadata = [](const FLAC__StreamDecoder* decoder, const FLAC__StreamMetadata* metadata, void* ctx) noexcept -> void
				{
					ZE_ASSERT(metadata, "Empty FLAC metadata!");
					ZE_ASSERT(ctx, "Empty FLAC context!");

					switch (metadata->type)
					{
					case FLAC__METADATA_TYPE_STREAMINFO:
					{
						auto& context = *reinterpret_cast<FlacCtx*>(ctx);

						context.ChannelCount = Utils::SafeCast<U8>(metadata->data.stream_info.channels);
						context.AudioInfo.Bytes = Utils::SafeCast<U32>(metadata->data.stream_info.total_samples * context.ChannelCount * Math::DivideRoundUp(metadata->data.stream_info.bits_per_sample, 8U));
						context.AudioInfo.SampleRate = metadata->data.stream_info.sample_rate;
						context.AudioInfo.BitsPerSample = Utils::SafeCast<U8>(metadata->data.stream_info.bits_per_sample);
						context.AudioInfo.IsFloat = false;
						break;
					}
					case FLAC__METADATA_TYPE_VORBIS_COMMENT:
					{
						for (U32 i = 0; i < metadata->data.vorbis_comment.num_comments; ++i)
						{
							auto& comment = metadata->data.vorbis_comment.comments[i];
							// Parse channel mask
							ChannelMask mask = Utils::ParseVorbisChannelMask(reinterpret_cast<const char*>(comment.entry), comment.length);
							if (mask != 0)
							{
								reinterpret_cast<FlacCtx*>(ctx)->AudioInfo.Channels = mask;
								break;
							}
						}
						break;
					}
					default:
						break;
					}
				};
			FLAC__StreamDecoderErrorCallback error = [](const FLAC__StreamDecoder* decoder, FLAC__StreamDecoderErrorStatus status, void* ctx) noexcept -> void
				{
					ZE_ASSERT(ctx, "Empty FLAC context!");
					// Just save the error for retrieval later
					reinterpret_cast<FlacCtx*>(ctx)->Code = ZE_FLAC_DECODER_ERROR(status);
				};

			// Parse only metadata and keep stream in state of waiting to parse sample data
			ZE_WARNING_DISABLE_MSVC(5039);
			FLAC__StreamDecoderInitStatus initStatus = FLAC__stream_decoder_init_stream(decoder, read, seek, tell, length, eof, write, metadata, error, &ctx);
			if (initStatus == FLAC__STREAM_DECODER_INIT_STATUS_OK)
			{
				if (FLAC__stream_decoder_process_until_end_of_metadata(decoder))
				{
					if (ctx.AudioInfo.Channels == 0)
					{
						// Fallback when no channel mask is provided
						ctx.AudioInfo.Channels = GetDefaultMask(ctx.ChannelCount);
						if (ctx.AudioInfo.Channels == 0)
							ctx.Code = ZE_FLAC_DECODER_ERROR(FLAC__STREAM_DECODER_ERROR_STATUS_BAD_METADATA);
					}
				}
				if (ctx.Code)
					FLAC__stream_decoder_finish(decoder);
			}
			else
				ctx.Code = ZE_FLAC_DECODER_INIT_ERROR(initStatus);

			if (ctx.Code)
			{
				FLAC__stream_decoder_delete(decoder);
				return std::unexpected(ctx.Code);
			}
			state.Flac.Decoder = decoder;
			break;
		}
		case FileSourceType::OggVorbis:
		{
			ZE_EXPECT_RET_FAILED(state.OggVorbis.Ctx, FileUtils::GetOggVorbisCtx(file, regionSize, audioInfo));
			break;
		}
		case FileSourceType::OggOpus:
		{
			ZE_EXPECT_RET_FAILED(state.OggOpus.Ctx, FileUtils::GetOggOpusCtx(file, regionSize, audioInfo));
			break;
		}
		}
		state.AudioBlockSize = Math::DivideRoundUp<U8>(audioInfo.BitsPerSample, 8) * Intrin::CountBitsSet(static_cast<U32>(audioInfo.Channels));
		state.AudioBlockSize = (StreamState::MAX_BUFFER_SIZE / state.AudioBlockSize) * state.AudioBlockSize;
		return state;
	}

	Task<Status> ReadNextChunk(StreamState& state, U32 readSize, U8* destBuffer, bool loopAudio) noexcept
	{
		ZE_ASSERT(destBuffer, "Empty output buffer!");

		switch (state.SourceType)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case FileSourceType::Unknown:
		{
			ZE_FAIL("Unknown audio file format!");
			co_return IO::WAV::Error::Make(IO::WAV::FileResult::Unknown);
		}
		case FileSourceType::WAV:
		{
			ZE_ASSERT(state.WAV.File, "WAV ctx not initialized!");
			Status stat = co_await state.WAV.File->ReadAsync(destBuffer, readSize, state.WAV.DataStart + state.WAV.CurrentPosition);
			if (stat)
			{
				if (IO::EofResult::IsEOF(stat))
				{
					U32 bytesRead = IO::EofResult::GetRealBytes(stat);
					if (loopAudio)
					{
						readSize -= bytesRead;
						stat = co_await state.WAV.File->ReadAsync(destBuffer + bytesRead, readSize, state.WAV.DataStart);
						if (stat)
							co_return stat;
						state.WAV.CurrentPosition = state.WAV.DataStart + readSize;
					}
					else
					{
						state.WAV.CurrentPosition += bytesRead;
						co_return stat;
					}
				}
				else
					co_return stat;
			}
			else
				state.WAV.CurrentPosition += readSize;
			break;
		}
		case FileSourceType::Flac:
		{
			break;
		}
		case FileSourceType::OggVorbis:
		{
			ZE_ASSERT(state.OggVorbis.Ctx.Decoder.callbacks.read_func, "Ogg vorbis ctx not initialized!");

			U32 bytesRead = 0;
			ZE_EXPECT_RET_FAILED_CODE_CORO(bytesRead, FileUtils::ReadOggVorbis(state.OggVorbis.Ctx,
				readSize, reinterpret_cast<float*>(destBuffer)));
			if (bytesRead < readSize)
			{
				if (loopAudio)
				{
					ZE_CODE_RET_FAILED_CORO(RewindToBegining(state));

					readSize -= bytesRead;
					ZE_EXPECT_RET_FAILED_CODE_CORO(bytesRead, FileUtils::ReadOggVorbis(state.OggVorbis.Ctx,
						readSize, reinterpret_cast<float*>(destBuffer + bytesRead)));
					if (bytesRead != readSize)
						co_return IO::EofResult::Make(bytesRead);
				}
				else
					co_return IO::EofResult::Make(bytesRead);
			}
			break;
		}
		case FileSourceType::OggOpus:
		{
			ZE_ASSERT(state.OggOpus.Ctx.Decoder, "Ogg opus ctx not initialized!");

			U32 bytesRead = 0;
			ZE_EXPECT_RET_FAILED_CODE_CORO(bytesRead, FileUtils::ReadOggOpus(state.OggOpus.Ctx,
				readSize, reinterpret_cast<float*>(destBuffer)));
			if (bytesRead < readSize)
			{
				if (loopAudio)
				{
					ZE_CODE_RET_FAILED_CORO(RewindToBegining(state));

					readSize -= bytesRead;
					ZE_EXPECT_RET_FAILED_CODE_CORO(bytesRead, FileUtils::ReadOggOpus(state.OggOpus.Ctx,
						readSize, reinterpret_cast<float*>(destBuffer + bytesRead)));
					if (bytesRead != readSize)
						co_return IO::EofResult::Make(bytesRead);
				}
				else
					co_return IO::EofResult::Make(bytesRead);
			}
			break;
		}
		}
		co_return Status{};
	}

	Status RewindToBegining(StreamState& state) noexcept
	{
		switch (state.SourceType)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case FileSourceType::Unknown:
		{
			ZE_FAIL("Unknown audio file format!");
			return IO::WAV::Error::Make(IO::WAV::FileResult::Unknown);
		}
		case FileSourceType::WAV:
		{
			ZE_ASSERT(state.WAV.File, "WAV ctx not initialized!");
			state.WAV.CurrentPosition = state.WAV.DataStart;
			break;
		}
		case FileSourceType::Flac:
		{
			if (!FLAC__stream_decoder_seek_absolute(state.Flac.Decoder, 0))
			{
				if (FLAC__stream_decoder_get_state(state.Flac.Decoder) == FLAC__STREAM_DECODER_SEEK_ERROR)
					FLAC__stream_decoder_flush(state.Flac.Decoder);
				//return ZE_FLAC_DECODER_ERROR(); // Get decoder error from state
			}
			break;
		}
		case FileSourceType::OggVorbis:
			ZE_ASSERT(state.OggVorbis.Ctx.Decoder.callbacks.read_func, "Ogg vorbis ctx not initialized!");
			return ZE_VORBIS_ERROR(ov_pcm_seek(&state.OggVorbis.Ctx.Decoder, 0));
		case FileSourceType::OggOpus:
			ZE_ASSERT(state.OggOpus.Ctx.Decoder, "Ogg opus ctx not initialized!");
			return ZE_OPUS_ERROR(op_pcm_seek(state.OggOpus.Ctx.Decoder, 0));
		}
		return {};
	}
}