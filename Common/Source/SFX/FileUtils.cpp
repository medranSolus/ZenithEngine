#include "SFX/FileUtils.h"

namespace ZE::SFX::FileUtils
{
	Expected<OggVorbisCtx> GetOggVorbisCtx(IO::File& file, U64 regionSize, AudioDesc& audioDesc) noexcept
	{
		OggVorbisCtx ctx = { &file, regionSize };
		ctx.Decoder.callbacks.read_func = [](void* buffer, size_t size, size_t count, void* ctx) noexcept -> size_t
			{
				ZE_ASSERT(ctx, "Empty vorbis file context!");

				auto& context = *reinterpret_cast<OggVorbisCtx*>(ctx);
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
		ctx.Decoder.callbacks.seek_func = [](void* ctx, ogg_int64_t offset, int whence) noexcept -> int
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
		ctx.Decoder.callbacks.close_func = nullptr;
		ctx.Decoder.callbacks.tell_func = [](void* ctx) noexcept -> long
			{
				ZE_ASSERT(ctx, "Empty vorbis file context!");
				return Utils::SafeCast<long>(reinterpret_cast<OggVorbisCtx*>(ctx)->ReadOffset);
			};

		ZE_CODE_RET_FAILED_EXPECT(ZE_VORBIS_ERROR(ov_open_callbacks(&ctx, &ctx.Decoder, nullptr, 0, ctx.Decoder.callbacks)));
		ZE_CODE_RET_FAILED_EXPECT(ctx.Code);

		vorbis_info* streamInfo = ov_info(&ctx.Decoder, -1);
		if (!streamInfo || streamInfo->channels > 8 && streamInfo->channels <= 0)
			return std::unexpected(ZE_VORBIS_ERROR(OV_FALSE));

		ctx.ChannelCount = Utils::SafeCast<U8>(streamInfo->channels);
		S64 samples = ov_pcm_total(&ctx.Decoder, -1);
		if (samples < 0)
			return std::unexpected(ZE_VORBIS_ERROR(Utils::SafeCast<S32>(samples)));

		audioDesc.Bytes = Utils::SafeCast<U32>(samples * sizeof(float) * ctx.ChannelCount);
		audioDesc.SampleRate = Utils::SafeCast<U32>(streamInfo->rate);
		// But requires remapping to correct order
		audioDesc.Channels = SFX::GetDefaultMask(ctx.ChannelCount);
		audioDesc.BitsPerSample = 32;
		audioDesc.IsFloat = true;

		return ctx;
	}

	Expected<U32> ReadOggVorbis(OggVorbisCtx& ctx, U32 bytes, float* interleavedSamples) noexcept
	{
		ZE_ASSERT(ctx.Decoder.callbacks.read_func, "Empty decoder handle!");
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
			samplesReadCount = ov_read_float(&ctx.Decoder, reinterpret_cast<float***>(&readSamples), requestedSamples, &currentStream);

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

	Expected<OggOpusCtx> GetOggOpusCtx(IO::File& file, U64 regionSize, AudioDesc& audioDesc) noexcept
	{
		OggOpusCtx ctx = { &file, regionSize };
		OpusFileCallbacks opusCallbacks = {};
		opusCallbacks.read = [](void* ctx, unsigned char* buffer, int bytes) noexcept -> int
			{
				ZE_ASSERT(ctx, "Empty opus file context!");

				auto& context = *reinterpret_cast<OggOpusCtx*>(ctx);
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

				auto& context = *reinterpret_cast<OggOpusCtx*>(ctx);
				if (whence == SEEK_END)
					offset = context.RegionSize - offset;
				else if (whence == SEEK_CUR)
					offset += context.ReadOffset;
				context.ReadOffset = Utils::SafeCast<U64>(offset);
				return 0;
			};
		opusCallbacks.tell = [](void* ctx) noexcept -> opus_int64
			{
				ZE_ASSERT(ctx, "Empty opus file context!");
				return Utils::SafeCast<opus_int64>(reinterpret_cast<OggOpusCtx*>(ctx)->ReadOffset);
			};
		opusCallbacks.close = nullptr;

		int error = 0;
		ctx.Decoder = op_open_callbacks(&ctx, &opusCallbacks, nullptr, 0, &error);
		if (!ctx.Decoder)
			return std::unexpected(ZE_OPUS_ERROR(error));
		ZE_ASSERT(op_link_count(ctx.Decoder) == 1, "By default only single link streams are supported!");

		ctx.ChannelCount = Utils::SafeCast<U8>(op_channel_count(ctx.Decoder, -1));
		S64 samples = op_pcm_total(ctx.Decoder, -1);
		if (samples < 0)
			return std::unexpected(ZE_OPUS_ERROR(Utils::SafeCast<S32>(samples)));

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
}