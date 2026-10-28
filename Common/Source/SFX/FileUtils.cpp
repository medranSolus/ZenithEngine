#include "SFX/FileUtils.h"
#include "SFX/FileError.h"

namespace ZE::SFX::FileUtils
{
	Expected<OggVorbisCtx> GetOggVorbisCtx(IO::File& file, U64 regionEnd, AudioDesc& audioDesc) noexcept
	{
		OggVorbisCtx ctx = { std::make_unique<OggVorbisCtx::CallbackCtx>(&file, regionEnd) };
		ctx.Decoder = &ctx.IoCtx->Decoder;
		ctx.Decoder->callbacks.read_func = [](void* buffer, size_t size, size_t count, void* ctx) noexcept -> size_t
			{
				ZE_ASSERT(ctx, "Empty vorbis file context!");

				auto& context = *reinterpret_cast<OggVorbisCtx::CallbackCtx*>(ctx);
				ZE_ASSERT(context.File, "Empty vorbis file handle!");

				U32 bytes = Utils::SafeCast<U32>(size * count);
				context.Code = context.File->Read(buffer, bytes, context.ReadOffset);

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
		ctx.Decoder->callbacks.seek_func = [](void* ctx, ogg_int64_t offset, int whence) noexcept -> int
			{
				ZE_ASSERT(ctx, "Empty vorbis file context!");

				auto& context = *reinterpret_cast<OggVorbisCtx::CallbackCtx*>(ctx);
				if (whence == SEEK_END)
					offset = context.FileEnd - offset;
				else if (whence == SEEK_CUR)
					offset += context.ReadOffset;
				context.ReadOffset = Utils::SafeCast<U64>(offset);
				return 0;
			};
		ctx.Decoder->callbacks.close_func = nullptr;
		ctx.Decoder->callbacks.tell_func = [](void* ctx) noexcept -> long
			{
				ZE_ASSERT(ctx, "Empty vorbis file context!");
				return Utils::SafeCast<long>(reinterpret_cast<OggVorbisCtx::CallbackCtx*>(ctx)->ReadOffset);
			};

		ZE_CODE_RET_FAILED_EXPECT(ZE_VORBIS_ERROR(ov_open_callbacks(ctx.IoCtx.get(), ctx.Decoder, nullptr, 0, ctx.Decoder->callbacks)));

		vorbis_info* streamInfo = ov_info(ctx.Decoder, -1);
		if (streamInfo && streamInfo->channels <= 8 && streamInfo->channels > 0)
		{
			ctx.ChannelCount = Utils::SafeCast<U8>(streamInfo->channels);
			S64 samples = ov_pcm_total(ctx.Decoder, -1);
			if (samples > 0)
			{
				audioDesc.Bytes = Utils::SafeCast<U32>(samples * sizeof(float) * ctx.ChannelCount);
				audioDesc.SampleRate = Utils::SafeCast<U32>(streamInfo->rate);
				// But requires remapping to correct order
				audioDesc.Channels = SFX::GetDefaultMask(ctx.ChannelCount);
				audioDesc.BitsPerSample = 32;
				audioDesc.IsFloat = true;
			}
			else
				ctx.IoCtx->Code = ZE_VORBIS_ERROR(Utils::SafeCast<S32>(samples));
		}
		else
			ctx.IoCtx->Code = ZE_VORBIS_ERROR(OV_FALSE);

		if (ctx.IoCtx->Code)
		{
			ov_clear(ctx.Decoder);
			return std::unexpected(ctx.IoCtx->Code);
		}
		return ctx;
	}

	Expected<U32> ReadOggVorbis(OggVorbisCtx& ctx, U32 bytes, float* interleavedSamples) noexcept
	{
		ZE_ASSERT(ctx.Decoder, "Empty decoder handle!");
		ZE_ASSERT(interleavedSamples, "Empty output samples buffer!");

		// Create final interleaved channel mappings
		U8 channelMapping[8] = {};
		switch (ctx.ChannelCount)
		{
		default:
		case 1:
		case 2:
		case 4:
		{
			// Channel order is the same
			for (U8 i = 0; i < ctx.ChannelCount; ++i)
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

			switch (ctx.ChannelCount)
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

		S32 requestedSamples = bytes / (sizeof(float) * ctx.ChannelCount);
		S32 samplesReadCount = 0;
		U32 writeOffset = 0;
		do
		{
			int currentStream = 0;
			float** readSamples = nullptr;
			samplesReadCount = ov_read_float(ctx.Decoder, reinterpret_cast<float***>(&readSamples), requestedSamples, &currentStream);

			if (samplesReadCount > 0 && readSamples)
			{
				requestedSamples -= samplesReadCount;
				writeOffset += samplesReadCount * ctx.ChannelCount * sizeof(float);
				for (S64 i = 0; i < samplesReadCount; ++i)
				{
					for (U8 c = 0; c < ctx.ChannelCount; ++c)
						interleavedSamples[channelMapping[c]] = readSamples[c][i];
					interleavedSamples += ctx.ChannelCount;
				}
			}
			else if (samplesReadCount == OV_HOLE)
				samplesReadCount = 1;
			else if (samplesReadCount < 0)
				return std::unexpected(ZE_VORBIS_ERROR(Utils::SafeCast<S32>(samplesReadCount)));
		} while (samplesReadCount > 0 && requestedSamples > 0);

		ZE_ASSERT(bytes >= writeOffset, "Shouldn't happen!");
		return writeOffset;
	}

	Expected<OggOpusCtx> GetOggOpusCtx(IO::File& file, U64 regionEnd, AudioDesc& audioDesc) noexcept
	{
		OggOpusCtx ctx = { std::make_unique<OggOpusCtx::CallbackCtx>(&file, regionEnd) };
		OpusFileCallbacks opusCallbacks = {};
		opusCallbacks.read = [](void* ctx, unsigned char* buffer, int bytes) noexcept -> int
			{
				ZE_ASSERT(ctx, "Empty opus file context!");

				auto& context = *reinterpret_cast<OggOpusCtx::CallbackCtx*>(ctx);
				ZE_ASSERT(context.File, "Empty opus file handle!");
				context.Code = context.File->Read(buffer, Utils::SafeCast<U32>(bytes), context.ReadOffset);

				if (context.Code)
				{
					if (IO::EofResult::IsEOF(context.Code))
					{
						bytes = Utils::SafeCast<int>(IO::EofResult::GetRealBytes(context.Code));
						context.ReadOffset += bytes;
						context.Code = {};
					}
					else
						bytes = 0;
				}
				else
					context.ReadOffset += bytes;
				return bytes;
			};
		opusCallbacks.seek = [](void* ctx, opus_int64 offset, int whence) noexcept -> int
			{
				ZE_ASSERT(ctx, "Empty opus file context!");

				auto& context = *reinterpret_cast<OggOpusCtx::CallbackCtx*>(ctx);
				if (whence == SEEK_END)
					offset = context.FileEnd - offset;
				else if (whence == SEEK_CUR)
					offset += context.ReadOffset;
				context.ReadOffset = Utils::SafeCast<U64>(offset);
				return 0;
			};
		opusCallbacks.tell = [](void* ctx) noexcept -> opus_int64
			{
				ZE_ASSERT(ctx, "Empty opus file context!");
				return Utils::SafeCast<opus_int64>(reinterpret_cast<OggOpusCtx::CallbackCtx*>(ctx)->ReadOffset);
			};
		opusCallbacks.close = nullptr;

		int error = 0;
		ctx.Decoder = op_open_callbacks(ctx.IoCtx.get(), &opusCallbacks, nullptr, 0, &error);
		if (!ctx.Decoder)
			return std::unexpected(ZE_OPUS_ERROR(error));
		ZE_ASSERT(op_link_count(ctx.Decoder) == 1, "By default only single link streams are supported!");

		ctx.ChannelCount = Utils::SafeCast<U8>(op_channel_count(ctx.Decoder, -1));
		S64 samples = op_pcm_total(ctx.Decoder, -1);
		if (samples < 0)
		{
			op_free(ctx.Decoder);
			return std::unexpected(ZE_OPUS_ERROR(Utils::SafeCast<S32>(samples)));
		}

		audioDesc.Bytes = Utils::SafeCast<U32>(samples * sizeof(float) * ctx.ChannelCount);
		audioDesc.SampleRate = 48000;
		// But requires remapping to correct order
		audioDesc.Channels = GetDefaultMask(ctx.ChannelCount);
		audioDesc.BitsPerSample = 32;
		audioDesc.IsFloat = true;

		return ctx;
	}

	Expected<U32> ReadOggOpus(OggOpusCtx& ctx, U32 bytes, float* interleavedSamples) noexcept
	{
		ZE_ASSERT(ctx.Decoder, "Empty decoder handle!");
		ZE_ASSERT(interleavedSamples, "Empty output samples buffer!");

		S32 bufferSpace = bytes / sizeof(float);
		S32 samplesReadCount = 0;
		U32 writeOffset = 0;
		float* readSamples = interleavedSamples;
		do
		{
			samplesReadCount = op_read_float(ctx.Decoder, readSamples, bufferSpace, nullptr);
			if (samplesReadCount > 0)
			{
				S32 offset = samplesReadCount * ctx.ChannelCount;
				bufferSpace -= offset;
				readSamples += offset;
				writeOffset += offset * sizeof(float);
			}
			else if (samplesReadCount == OP_HOLE)
				samplesReadCount = 1;
			else if (samplesReadCount < 0)
				return std::unexpected(ZE_OPUS_ERROR(samplesReadCount));
		} while (samplesReadCount > 0 && bufferSpace > 0);

		// Change to expected interleaving of samples (same order as for vorbis)
		switch (ctx.ChannelCount)
		{
		default:
		case 1:
		case 2:
		case 4:
			// Same channel order
			break;
		case 3:
		case 5:
		case 6:
		case 7:
		case 8:
		{
			for (S64 i = 0, samples = bytes / (sizeof(float) * ctx.ChannelCount); i < samples; ++i)
			{
				// All require swapping FC and FR order
				std::swap(interleavedSamples[1], interleavedSamples[2]);

				switch (ctx.ChannelCount)
				{
				default:
				case 3:
				case 5:
					break;
				case 6:
				{
					float lfe = interleavedSamples[3];
					interleavedSamples[3] = interleavedSamples[4];
					interleavedSamples[4] = interleavedSamples[5];
					interleavedSamples[5] = lfe;
					break;
				}
				case 7:
				{
					float sr = interleavedSamples[4];
					interleavedSamples[4] = interleavedSamples[5];
					interleavedSamples[5] = interleavedSamples[3];
					interleavedSamples[3] = interleavedSamples[6];
					interleavedSamples[6] = sr;
					break;
				}
				case 8:
				{
					float sr = interleavedSamples[4];
					interleavedSamples[4] = interleavedSamples[5];
					interleavedSamples[5] = interleavedSamples[6];
					interleavedSamples[6] = interleavedSamples[3];
					interleavedSamples[3] = interleavedSamples[7];
					interleavedSamples[7] = sr;
					break;
				}
				}
				interleavedSamples += ctx.ChannelCount;
			}
			break;
		}
		}
		return writeOffset;
	}

	Expected<FlacCtx> GetFlacCtx(IO::File& file, U64 startOffset, U64 regionSize, AudioDesc& audioDesc) noexcept
	{
		FlacCtx ctx = { std::make_unique<FlacCtx::CallbackCtx>(&file, startOffset + regionSize, regionSize) };
		ctx.Decoder = FLAC__stream_decoder_new();
		if (!ctx.Decoder)
		{
			ZE_FAIL("Failed to create FLAC decoder!");
			return std::unexpected(ZE_FLAC_DECODER_INIT_ERROR(FLAC__STREAM_DECODER_INIT_STATUS_MEMORY_ALLOCATION_ERROR));
		}
		FLAC__stream_decoder_set_md5_checking(ctx.Decoder, _ZE_MODE_DEBUG);
		FLAC__stream_decoder_set_metadata_respond(ctx.Decoder, FLAC__METADATA_TYPE_STREAMINFO);
		FLAC__stream_decoder_set_metadata_respond(ctx.Decoder, FLAC__METADATA_TYPE_VORBIS_COMMENT);

		FLAC__StreamDecoderReadCallback read = [](const FLAC__StreamDecoder* decoder, FLAC__byte buffer[], size_t* bytes, void* ctx) noexcept -> FLAC__StreamDecoderReadStatus
			{
				ZE_ASSERT(ctx, "Empty FLAC context!");
				ZE_ASSERT(bytes, "Empty FLAC byte count!");
				ZE_ASSERT(buffer, "Empty FLAC output buffer!");

				if (*bytes > 0)
				{
					auto& context = *reinterpret_cast<FlacCtx::CallbackCtx*>(ctx);
					context.Code = context.File->Read(buffer, Utils::SafeCast<U32>(*bytes), context.ReadOffset);

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

				reinterpret_cast<FlacCtx::CallbackCtx*>(ctx)->ReadOffset = offset;
				return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
			};
		FLAC__StreamDecoderTellCallback tell = [](const FLAC__StreamDecoder* decoder, FLAC__uint64* offset, void* ctx) noexcept -> FLAC__StreamDecoderTellStatus
			{
				ZE_ASSERT(ctx, "Empty FLAC context!");
				ZE_ASSERT(offset, "Empty FLAC offset!");

				*offset = reinterpret_cast<FlacCtx::CallbackCtx*>(ctx)->ReadOffset;
				return FLAC__STREAM_DECODER_TELL_STATUS_OK;
			};
		FLAC__StreamDecoderLengthCallback length = [](const FLAC__StreamDecoder* decoder, FLAC__uint64* streamLen, void* ctx) noexcept -> FLAC__StreamDecoderLengthStatus
			{
				ZE_ASSERT(ctx, "Empty FLAC context!");
				ZE_ASSERT(streamLen, "Empty stream length!");

				*streamLen = reinterpret_cast<FlacCtx::CallbackCtx*>(ctx)->RegionSize;
				return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
			};
		FLAC__StreamDecoderEofCallback eof = [](const FLAC__StreamDecoder* decoder, void* ctx) noexcept -> FLAC__bool
			{
				ZE_ASSERT(ctx, "Empty FLAC context!");

				auto& context = *reinterpret_cast<FlacCtx::CallbackCtx*>(ctx);
				return context.ReadOffset >= context.FileEnd;
			};
		FLAC__StreamDecoderWriteCallback write = [](const FLAC__StreamDecoder* decoder, const FLAC__Frame* frame, const FLAC__int32* const buffer[], void* ctx) noexcept -> FLAC__StreamDecoderWriteStatus
			{
				ZE_ASSERT(frame, "Empty FLAC frame!");
				ZE_ASSERT(buffer, "Empty FLAC buffer!");
				ZE_ASSERT(ctx, "Empty FLAC context!");

				auto& context = *reinterpret_cast<FlacCtx::CallbackCtx*>(ctx);
				const U8 sampleChannelBytes = Math::DivideRoundUp<U8>(context.Desc.BitsPerSample, 8);
				const U32 sampleBytes = sampleChannelBytes * context.ChannelCount;

				// Get any data from last frame that did not fit into the buffer
				if (context.RolloverBuffer.size())
				{
					context.WriteOffset = std::min(Utils::SafeCast<U32>(context.RolloverBuffer.size()), context.BufferSize);
					std::memcpy(context.OutBuffer, context.RolloverBuffer.data(), context.WriteOffset);
					context.RolloverBuffer.resize(context.RolloverBuffer.size() - context.WriteOffset);
				}

				// Copy into interleaved format
				U32 i = 0;
				for (; i < frame->header.blocksize; ++i)
				{
					// Move into data cache
					if (context.WriteOffset + sampleBytes > context.BufferSize)
					{
						context.WriteOffset = Utils::SafeCast<U32>(context.RolloverBuffer.size());
						context.RolloverBuffer.resize(context.WriteOffset + (frame->header.blocksize - i) * sampleBytes);
						break;
					}
					for (U8 j = 0; j < context.ChannelCount; ++j)
					{
						std::memcpy(context.OutBuffer + context.WriteOffset, buffer[j] + i, sampleChannelBytes);
						context.WriteOffset += sampleChannelBytes;
					}
				}

				// Copy remainder of frame into the cache for later decoding
				for (; i < frame->header.blocksize; ++i)
				{
					for (U8 j = 0; j < context.ChannelCount; ++j)
					{
						std::memcpy(context.RolloverBuffer.data() + context.WriteOffset, buffer[j] + i, sampleChannelBytes);
						context.WriteOffset += sampleChannelBytes;
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
				default:
					break;
				case FLAC__METADATA_TYPE_STREAMINFO:
				{
					auto& context = *reinterpret_cast<FlacCtx::CallbackCtx*>(ctx);

					context.ChannelCount = Utils::SafeCast<U8>(metadata->data.stream_info.channels);
					context.Desc.Bytes = Utils::SafeCast<U32>(metadata->data.stream_info.total_samples * context.ChannelCount * Math::DivideRoundUp(metadata->data.stream_info.bits_per_sample, 8U));
					context.Desc.SampleRate = metadata->data.stream_info.sample_rate;
					context.Desc.BitsPerSample = Utils::SafeCast<U8>(metadata->data.stream_info.bits_per_sample);
					context.Desc.IsFloat = false;
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
							reinterpret_cast<FlacCtx::CallbackCtx*>(ctx)->Desc.Channels = mask;
							break;
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
				auto& context = *reinterpret_cast<FlacCtx::CallbackCtx*>(ctx);
				if (context.Code)
				{
					ZE_CODE_ERROR(context.Code, "Previous Flac decoder error");
				}
				context.Code = ZE_FLAC_DECODER_ERROR(status);
			};

		ZE_WARNING_DISABLE_MSVC(5039);
		FLAC__StreamDecoderInitStatus initStatus = FLAC__stream_decoder_init_stream(ctx.Decoder, read, seek, tell, length, eof, write, metadata, error, ctx.IoCtx.get());
		if (initStatus == FLAC__STREAM_DECODER_INIT_STATUS_OK)
		{
			FLAC__stream_decoder_process_until_end_of_metadata(ctx.Decoder);
			// Won't be needed further on
			FLAC__stream_decoder_set_metadata_ignore_all(ctx.Decoder);

			audioDesc = ctx.IoCtx->Desc;
			if (audioDesc.Channels == 0)
			{
				// Fallback when no channel mask is provided
				audioDesc.Channels = GetDefaultMask(ctx.IoCtx->ChannelCount);
				if (audioDesc.Channels == 0)
					ctx.IoCtx->Code = ZE_FLAC_DECODER_ERROR(FLAC__STREAM_DECODER_ERROR_STATUS_BAD_METADATA);
			}
		}
		else
			ctx.IoCtx->Code = ZE_FLAC_DECODER_INIT_ERROR(initStatus);

		if (ctx.IoCtx->Code)
		{
			FLAC__stream_decoder_delete(ctx.Decoder);
			return std::unexpected(ctx.IoCtx->Code);
		}
		return ctx;
	}
}