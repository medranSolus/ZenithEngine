#include "SFX/FileUtils.h"

namespace ZE::SFX::FileUtils
{
	Expected<AudioDesc> GetOggVorbisInfo(OggVorbis_File& decoder) noexcept
	{
		vorbis_info* streamInfo = ov_info(&decoder, -1);
		if (!streamInfo || streamInfo->channels > 8 && streamInfo->channels <= 0)
			return std::unexpected(ZE_VORBIS_ERROR(OV_FALSE));

		S64 samples = ov_pcm_total(&decoder, -1);
		if (samples < 0)
			return std::unexpected(ZE_VORBIS_ERROR(Utils::SafeCast<S32>(samples)));

		AudioDesc desc = {};
		desc.Bytes = Utils::SafeCast<U32>(samples * sizeof(float) * streamInfo->channels);
		desc.SampleRate = Utils::SafeCast<U32>(streamInfo->rate);
		// But requires remapping to correct order
		desc.Channels = SFX::GetDefaultMask(Utils::SafeCast<U8>(streamInfo->channels));
		desc.BitsPerSample = 32;
		desc.IsFloat = true;
		return desc;
	}

	Expected<U32> ReadOggVorbis(OggVorbis_File& decoder, U32 bytes, ChannelMask mask, float* interleavedSamples) noexcept
	{
		ZE_ASSERT(interleavedSamples, "Empty output samples buffer!");
		const U8 channelCount = Intrin::CountBitsSet(static_cast<U32>(mask));

		// Create final interleaved channel mappings
		U8 channelMapping[8] = {};
		switch (channelCount)
		{
		default:
		case 1:
		case 2:
		case 4:
		{
			// Channel order is the same
			for (U8 i = 0; i < channelCount; ++i)
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

			switch (channelCount)
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

		S32 requestedSamples = bytes / (sizeof(float) * channelCount);
		S32 samplesReadCount = 0;
		U32 writeOffset = 0;
		do
		{
			int currentStream = 0;
			float** readSamples = nullptr;
			samplesReadCount = ov_read_float(&decoder, reinterpret_cast<float***>(&readSamples), requestedSamples, &currentStream);

			if (samplesReadCount > 0 && readSamples)
			{
				requestedSamples -= samplesReadCount;
				writeOffset += samplesReadCount * channelCount * sizeof(float);
				for (S64 i = 0; i < samplesReadCount; ++i)
				{
					for (U8 c = 0; c < channelCount; ++c)
						interleavedSamples[channelMapping[c]] = readSamples[c][i];
					interleavedSamples += channelCount;
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

	Expected<AudioDesc> GetOggOpusInfo(OggOpusFile* decoder) noexcept
	{
		ZE_ASSERT(decoder, "Empty decoder handle!");
		ZE_ASSERT(op_link_count(decoder) == 1, "By default only single link streams are supported!");

		U8 channelCount = Utils::SafeCast<U8>(op_channel_count(decoder, -1));
		S64 samples = op_pcm_total(decoder, -1);
		if (samples < 0)
			return std::unexpected(ZE_OPUS_ERROR(Utils::SafeCast<S32>(samples)));

		AudioDesc desc = {};
		desc.Bytes = Utils::SafeCast<U32>(samples * sizeof(float) * channelCount);
		desc.SampleRate = 48000;
		// But requires remapping to correct order
		desc.Channels = GetDefaultMask(channelCount);
		desc.BitsPerSample = 32;
		desc.IsFloat = true;
		return desc;
	}

	Expected<U32> ReadOggOpus(OggOpusFile* decoder, U32 bytes, ChannelMask mask, float* interleavedSamples) noexcept
	{
		ZE_ASSERT(decoder, "Empty decoder handle!");
		ZE_ASSERT(interleavedSamples, "Empty output samples buffer!");
		const U8 channelCount = Intrin::CountBitsSet(static_cast<U32>(mask));

		S32 bufferSpace = bytes / sizeof(float);
		S32 samplesReadCount = 0;
		U32 writeOffset = 0;
		float* readSamples = interleavedSamples;
		do
		{
			samplesReadCount = op_read_float(decoder, readSamples, bufferSpace, nullptr);
			if (samplesReadCount > 0)
			{
				S32 offset = samplesReadCount * channelCount;
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
		switch (channelCount)
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
			for (S64 i = 0, samples = bytes / (sizeof(float) * channelCount); i < samples; ++i)
			{
				// All require swapping FC and FR order
				std::swap(interleavedSamples[1], interleavedSamples[2]);

				switch (channelCount)
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
				interleavedSamples += channelCount;
			}
			break;
		}
		}
		return writeOffset;
	}
}