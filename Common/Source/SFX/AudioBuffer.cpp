#include "SFX/AudioBuffer.h"
#include "SFX/FileError.h"
#include "IO/WAV/Utils.h"
ZE_WARNING_PUSH
#include "vorbis/vorbisfile.h"
#include "opusfile.h"
ZE_WARNING_POP

namespace ZE::SFX
{
	Expected<AudioBuffer> LoadFile(IO::File& file, U64 startOffset, U64 regionSize, FileSourceType type) noexcept
	{
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
			buffer.Samples = std::make_shared<U8[]>(buffer.Desc.Bytes);
			ZE_CODE_RET_FAILED_EXPECT(IO::WAV::LoadSampleData(file, dataStart, buffer.Samples.get(), buffer.Desc.Bytes, 0));
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
				AudioBuffer& Buffer;
				U64 StartOffset = 0;
				U64 RegionSize = 0;
				Status Code;
				U8 ChannelCount = 0;
				U32 WriteOffset = 0;
				U64 ReadOffset = 0;
			};

			FlacCtx ctx = { file, buffer, startOffset, regionSize };
			FLAC__StreamDecoderReadCallback read = [](const FLAC__StreamDecoder* decoder, FLAC__byte buffer[], size_t* bytes, void* ctx) noexcept -> FLAC__StreamDecoderReadStatus
				{
					ZE_ASSERT(ctx, "Empty FLAC context!");
					ZE_ASSERT(bytes, "Empty FLAC byte count!");
					ZE_ASSERT(buffer, "Empty FLAC output buffer!");

					if (*bytes > 0)
					{
						auto& context = *reinterpret_cast<FlacCtx*>(ctx);
						context.Code = context.File.Read(buffer, Utils::SafeCast<U32>(*bytes), context.ReadOffset);

						if (context.Code)
						{
							if (IO::EofResult::IsEOF(context.Code))
							{
								*bytes = IO::EofResult::GetRealBytes(context.Code);
								context.Code = {};
								context.ReadOffset += *bytes;
								return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
							}
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

					*streamLen = reinterpret_cast<FlacCtx*>(ctx)->RegionSize;
					return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
				};
			FLAC__StreamDecoderEofCallback eof = [](const FLAC__StreamDecoder* decoder, void* ctx) noexcept -> FLAC__bool
				{
					ZE_ASSERT(ctx, "Empty FLAC context!");

					auto& context = *reinterpret_cast<FlacCtx*>(ctx);
					return context.ReadOffset >= context.StartOffset + context.RegionSize;
				};
			FLAC__StreamDecoderWriteCallback write = [](const FLAC__StreamDecoder* decoder, const FLAC__Frame* frame, const FLAC__int32* const buffer[], void* ctx) noexcept -> FLAC__StreamDecoderWriteStatus
				{
					ZE_ASSERT(frame, "Empty FLAC frame!");
					ZE_ASSERT(buffer, "Empty FLAC buffer!");
					ZE_ASSERT(ctx, "Empty FLAC context!");

					auto& context = *reinterpret_cast<FlacCtx*>(ctx);

					// Sanity check
					const U8 sampleBytes = Math::DivideRoundUp<U8>(context.Buffer.Desc.BitsPerSample, 8);
					if (context.Buffer.Desc.Bytes < context.WriteOffset + (frame->header.blocksize * context.ChannelCount * sampleBytes))
						return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;

					// Copy into interleaved format
					for (U32 i = 0; i < frame->header.blocksize; ++i)
					{
						for (U8 j = 0; j < context.ChannelCount; ++j)
						{
							std::memcpy(context.Buffer.Samples.get() + context.WriteOffset, buffer[j] + i, sampleBytes);
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
						ZE_ASSERT(context.Buffer.Samples == nullptr, "FLAC STREAMINFO metadata already processed!");

						context.ChannelCount = Utils::SafeCast<U8>(metadata->data.stream_info.channels);
						context.Buffer.Desc.Bytes = Utils::SafeCast<U32>(metadata->data.stream_info.total_samples * context.ChannelCount * Math::DivideRoundUp(metadata->data.stream_info.bits_per_sample, 8U));
						context.Buffer.Desc.SampleRate = metadata->data.stream_info.sample_rate;
						context.Buffer.Desc.BitsPerSample = Utils::SafeCast<U8>(metadata->data.stream_info.bits_per_sample);
						context.Buffer.Desc.IsFloat = false;
						context.Buffer.Samples = std::make_shared<U8[]>(context.Buffer.Desc.Bytes);
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
								reinterpret_cast<FlacCtx*>(ctx)->Buffer.Desc.Channels = mask;
								break;
							}
						}
						break;
					}
					default:
					{
						if (metadata->is_last)
						{
							auto& context = *reinterpret_cast<FlacCtx*>(ctx);

							if (context.Buffer.Samples == nullptr)
							{
								ZE_FAIL("FLAC STREAMINFO metadata has not been processed!");
								context.Code = ZE_FLAC_DECODER_ERROR(FLAC__STREAM_DECODER_ERROR_STATUS_BAD_METADATA);
							}
						}
						break;
					}
					}
				};
			FLAC__StreamDecoderErrorCallback error = [](const FLAC__StreamDecoder* decoder, FLAC__StreamDecoderErrorStatus status, void* ctx) noexcept -> void
				{
					ZE_ASSERT(ctx, "Empty FLAC context!");
					// Just save the error for retrieval later
					reinterpret_cast<FlacCtx*>(ctx)->Code = ZE_FLAC_DECODER_ERROR(status);
				};

			ZE_WARNING_DISABLE_MSVC(5039);
			FLAC__StreamDecoderInitStatus initStatus = FLAC__stream_decoder_init_stream(decoder, read, seek, tell, length, eof, write, metadata, error, &ctx);
			if (initStatus == FLAC__STREAM_DECODER_INIT_STATUS_OK)
			{
				FLAC__stream_decoder_process_until_end_of_metadata(decoder);

				if (ctx.Buffer.Desc.Channels == 0)
				{
					// Fallback when no channel mask is provided
					ctx.Buffer.Desc.Channels = GetDefaultMask(ctx.ChannelCount);
					if (ctx.Buffer.Desc.Channels == 0)
						ctx.Code = ZE_FLAC_DECODER_ERROR(FLAC__STREAM_DECODER_ERROR_STATUS_BAD_METADATA);
				}

				if (!ctx.Code)
					FLAC__stream_decoder_process_until_end_of_stream(decoder);
				FLAC__stream_decoder_finish(decoder);
			}
			else
				ctx.Code = ZE_FLAC_DECODER_INIT_ERROR(initStatus);

			FLAC__stream_decoder_delete(decoder);
			if (ctx.Code)
				return std::unexpected(ctx.Code);
			break;
		}
		case FileSourceType::OggVorbis:
		{
			struct OggVorbisCtx
			{
				IO::File& File;
				U64 RegionSize = 0;
				Status Code;
				U64 ReadOffset = 0;
			};

			OggVorbisCtx ctx = { file, regionSize };
			OggVorbis_File decoder = {};
			decoder.callbacks.read_func = [](void* buffer, size_t size, size_t count, void* ctx) noexcept -> size_t
				{
					ZE_ASSERT(ctx, "Empty vorbis file context!");

					auto& context = *reinterpret_cast<OggVorbisCtx*>(ctx);
					U32 bytes = Utils::SafeCast<U32>(size * count);
					context.Code = context.File.Read(buffer, bytes, context.ReadOffset);

					if (context.Code)
					{
						if (IO::EofResult::IsEOF(context.Code))
						{
							bytes = IO::EofResult::GetRealBytes(context.Code);
							context.ReadOffset += bytes;
							count = bytes / size;
							context.Code = {};
						}
						else
							count = 0;
					}
					else
						context.ReadOffset += bytes;
					return count;
				};
			decoder.callbacks.seek_func = [](void* ctx, ogg_int64_t offset, int whence) noexcept -> int
				{
					ZE_ASSERT(ctx, "Empty vorbis file context!");

					auto& context = *reinterpret_cast<OggVorbisCtx*>(ctx);
					if (whence == SEEK_END)
						offset = context.RegionSize - offset;
					else if (whence == SEEK_CUR)
						offset += context.ReadOffset;
					context.ReadOffset = Utils::SafeCast<U64>(offset);
					return 0;
				};
			decoder.callbacks.close_func = nullptr;
			decoder.callbacks.tell_func = [](void* ctx) noexcept -> long
				{
					ZE_ASSERT(ctx, "Empty vorbis file context!");
					return Utils::SafeCast<long>(reinterpret_cast<OggVorbisCtx*>(ctx)->ReadOffset);
				};

			ZE_CODE_RET_FAILED_EXPECT(ZE_VORBIS_ERROR(ov_open_callbacks(&ctx, &decoder, nullptr, 0, decoder.callbacks)));
			if (!ctx.Code)
			{
				vorbis_info* streamInfo = ov_info(&decoder, -1);
				if (streamInfo && streamInfo->channels <= 8 && streamInfo->channels > 0)
				{
					buffer.Desc.SampleRate = Utils::SafeCast<U32>(streamInfo->rate);
					// But requires remapping to correct order
					buffer.Desc.Channels = SFX::GetDefaultMask(Utils::SafeCast<U8>(streamInfo->channels));
					buffer.Desc.BitsPerSample = 32;
					buffer.Desc.IsFloat = true;

					S64 samples = ov_pcm_total(&decoder, -1);
					if (samples > 0)
					{
						buffer.Desc.Bytes = Utils::SafeCast<U32>(samples * sizeof(float) * streamInfo->channels);
						buffer.Samples = std::make_shared<U8[]>(buffer.Desc.Bytes);

						// Create final interleaved channel mappings
						U8 channelMapping[8] = {};
						switch (streamInfo->channels)
						{
						default:
						case 1:
						case 2:
						case 4:
						{
							// Channel order is the same
							for (U8 i = 0; i < Utils::SafeCast<U8>(streamInfo->channels); ++i)
								channelMapping[i] = i;
							break;
						}
						case 3:
						case 5:
						case 6:
						case 7:
						case 8:
						{
							// All require swapping FC and FR order
							channelMapping[0] = 0;
							channelMapping[1] = 2;
							channelMapping[2] = 1;

							switch (streamInfo->channels)
							{
							default:
							case 3:
								// Source: FL, FC, FR
								// Dest:   FL, FR, FC
								break;
							case 5:
							{
								// Source: FL, FC, FR, BL, BR
								// Dest:   FL, FR, FC, BL, BR
								channelMapping[3] = 3;
								channelMapping[4] = 4;
								break;
							}
							case 6:
							{
								// Source: FL, FC, FR, BL, BR, LFE
								// Dest:   FL, FR, FC, LFE, BL, BR
								channelMapping[3] = 4;
								channelMapping[4] = 5;
								channelMapping[5] = 3;
								break;
							}
							case 7:
							{
								// Source: FL, FC, FR, SL, SR, BC, LFE
								// Dest:   FL, FR, FC, LFE, BC, SL, SR
								channelMapping[3] = 5;
								channelMapping[4] = 6;
								channelMapping[5] = 4;
								channelMapping[6] = 3;
								break;
							}
							case 8:
							{
								// Source: FL, FC, FR, SL, SR, BL, BR, LFE
								// Dest:   FL, FR, FC, LFE, BL, BR, SL, SR
								channelMapping[3] = 6;
								channelMapping[4] = 7;
								channelMapping[5] = 4;
								channelMapping[6] = 5;
								channelMapping[7] = 3;
								break;
							}
							}
							break;
						}
						}

						S64 samplesRead = 0;
						U32 writeOffset = 0;
						float* interleavedSamples = reinterpret_cast<float*>(buffer.Samples.get());
						do
						{
							int currentStream = 0;
							float** readSamples = nullptr;
							samplesRead = ov_read_float(&decoder, reinterpret_cast<float***>(&readSamples), 4096, &currentStream);
							if (samplesRead > 0 && readSamples)
							{
								for (S64 i = 0; i < samplesRead; ++i)
								{
									for (int c = 0; c < streamInfo->channels; ++c)
										interleavedSamples[channelMapping[c]] = readSamples[c][i];
									interleavedSamples += streamInfo->channels;
									writeOffset += streamInfo->channels * sizeof(float);
								}
							}
							else if (samplesRead == OV_HOLE)
								samplesRead = 1;
							else if (samplesRead < 0)
								ctx.Code = ZE_VORBIS_ERROR(Utils::SafeCast<S32>(samplesRead));
						} while (samplesRead > 0);
						ZE_ASSERT(buffer.Desc.Bytes >= writeOffset, "Shouldn't happen!");

						// Re-fit buffer to proper size
						if (writeOffset < buffer.Desc.Bytes)
						{
							auto audioBuffer = std::make_shared<U8[]>(writeOffset);
							std::memcpy(audioBuffer.get(), buffer.Samples.get(), writeOffset);
							buffer.Samples = std::move(audioBuffer);
							buffer.Desc.Bytes = writeOffset;
							samples = writeOffset / (2 * streamInfo->channels);
						}
					}
					else
						ctx.Code = ZE_VORBIS_ERROR(Utils::SafeCast<S32>(samples));
				}
				else
					ctx.Code = ZE_VORBIS_ERROR(OV_FALSE);
				ov_clear(&decoder);
			}

			if (ctx.Code)
				return std::unexpected(ctx.Code);
			break;
		}
		case FileSourceType::OggOpus:
		{
			struct OggOpusCtx
			{
				IO::File& File;
				U64 RegionSize = 0;
				Status Code;
				U64 ReadOffset = 0;
			};

			OggOpusCtx ctx = { file, regionSize };
			OpusFileCallbacks opusCallbacks = {};
			opusCallbacks.read = [](void* ctx, unsigned char* buffer, int bufferSize) noexcept -> int
				{
					return 0;
				};
			opusCallbacks.seek = [](void* ctx, opus_int64 offset, int whence) noexcept -> int
				{
					return 0;
				};
			opusCallbacks.tell = [](void* ctx) noexcept -> opus_int64
				{
					return 0;
				};
			opusCallbacks.close = nullptr;

			int error = 0;
			OggOpusFile* decoder = op_open_callbacks(&ctx, &opusCallbacks, nullptr, 0, &error);
			if (decoder)
			{
				ZE_ASSERT(op_link_count(decoder) == 1, "By default only single link streams are supported!");
				
				S64 samples = op_pcm_total(decoder, -1);
				if (samples > 0)
				{
					U8 channelCount = Utils::SafeCast<U8>(op_channel_count(decoder, -1));

					buffer.Desc.Bytes = Utils::SafeCast<U32>(samples * channelCount);
					buffer.Desc.SampleRate = 48000;
					buffer.Desc.Channels = GetDefaultMask(channelCount);
					buffer.Desc.BitsPerSample = 32;
					buffer.Desc.IsFloat = true;
				}
				else
					ctx.Code = ZE_OPUS_ERROR(Utils::SafeCast<S32>(samples));

				op_free(decoder);
			}
			else
				ctx.Code = ZE_OPUS_ERROR(error);

			if (ctx.Code)
				return std::unexpected(ctx.Code);
			break;
		}
		}
		return buffer;
	}
}