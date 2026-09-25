#include "IO/WAV/Utils.h"

namespace ZE::IO::WAV
{
	Expected<SFX::AudioDesc> ParseFileInfo(File& file, U64& dataStartOffset, U64 currentOffset) noexcept
	{
		auto checkRead = [&](void* item, U32 size) noexcept -> Status
		{
			Status status = file.Read(item, size, currentOffset);
			if (status)
			{
				if (EofResult::IsEOF(status))
				{
					currentOffset += EofResult::GetRealBytes(status);
					status = {};
				}
			}
			else
				currentOffset += size;
			return status;
		};
#define ZE_WAV_CHECK_READ(item) ZE_CODE_RET_FAILED_EXPECT(checkRead(&item, sizeof(item)))

		// Load all headers and check their validity
		RiffChunkHeader riffChunk = {};
		ZE_WAV_CHECK_READ(riffChunk);

		if (riffChunk.FileTypeMagicNumber != RIFF_MAGIC_NUMBER)
			return std::unexpected(Error::Make(FileResult::IncorrectMagicNumberRIFF));
		if (riffChunk.FileFormatMagicNumber != WAVE_MAGIC_NUMBER)
			return std::unexpected(Error::Make(FileResult::IncorrectMagicNumberWAVE));
		if (riffChunk.FileSize < sizeof(RiffChunkHeader) + sizeof(FormatChunkHeader) + sizeof(DataChunkHeader) - 8)
			return std::unexpected(Error::Make(FileResult::FileTooSmall));

		FormatExtensionChunkHeader formatChunk = {};
		ZE_WAV_CHECK_READ(formatChunk.FormatEx.Format);

		if (formatChunk.FormatEx.Format.MagicNumber != FORMAT_MAGIC_NUMBER)
			return std::unexpected(Error::Make(FileResult::IncorrectMagicNumberFormatChunk));

		switch (formatChunk.FormatEx.Format.Size)
		{
		case Base(FormatHeaderSize::Legacy):
			formatChunk.FormatEx.ExtensionSize = Base(FormatExtensionHeaderSize::None);
			break;
		case Base(FormatHeaderSize::Default):
		{
			ZE_WAV_CHECK_READ(formatChunk.FormatEx.ExtensionSize);
			break;
		}
		case Base(FormatHeaderSize::Extended):
		{
			if (formatChunk.FormatEx.Format.AudioFormat != FormatTag::Extended)
				return std::unexpected(Error::Make(FileResult::IncorrectAudioFormat));
			ZE_CODE_RET_FAILED_EXPECT(checkRead(&formatChunk.FormatEx.ExtensionSize, sizeof(FormatExtensionChunkHeader) - sizeof(FormatChunkHeader)));
			break;
		}
		default:
			return std::unexpected(Error::Make(FileResult::FormatChunkTooSmall));
		}

		// Parse audio description
		SFX::AudioDesc buffer = {};
		buffer.SampleRate = formatChunk.FormatEx.Format.SampleRate;

		switch (formatChunk.FormatEx.ExtensionSize)
		{
		case Base(FormatExtensionHeaderSize::None):
		{
			buffer.Channels = SFX::GetDefaultMask(Utils::SafeCast<U8>(formatChunk.FormatEx.Format.NumChannels));
			if (buffer.Channels == 0)
				return std::unexpected(Error::Make(FileResult::IncorrectAudioFormat));

			buffer.BitsPerSample = Utils::SafeCast<U8>(formatChunk.FormatEx.Format.BitsPerSample);
			switch (formatChunk.FormatEx.Format.AudioFormat)
			{
			case Base(FormatTag::PCM):
				buffer.IsFloat = false;
				break;
			case Base(FormatTag::Float):
			{
				buffer.IsFloat = true;
				if (buffer.BitsPerSample != 32)
					return std::unexpected(Error::Make(FileResult::IncorrectAudioFormat));
				break;
			}
			case Base(FormatTag::Extended):
			default:
				return std::unexpected(Error::Make(FileResult::IncorrectAudioFormat));
			}
			break;
		}
		case Base(FormatExtensionHeaderSize::Extensible):
		{
			// Channel mask is based on the same values as in WAVEFORMATEXTENSIBLE
			buffer.Channels = formatChunk.ChannelMask;
			buffer.BitsPerSample = Utils::SafeCast<U8>(formatChunk.BitsPerSample);
			buffer.IsFloat = buffer.BitsPerSample == 32;
			break;
		}
		default:
			return std::unexpected(Error::Make(FileResult::UnknownFormatChunkExtension));
		}

		// Locate data chunk
		DataChunkHeader dataChunk = {};
		ZE_WAV_CHECK_READ(dataChunk);

		while (dataChunk.MagicNumber != DATA_MAGIC_NUMBER)
		{
			currentOffset += dataChunk.SampleDataSize;
			ZE_WAV_CHECK_READ(dataChunk);
		}

		dataStartOffset = currentOffset;
		buffer.Bytes = dataChunk.SampleDataSize;
		return buffer;
#undef ZE_WAV_CHECK_READ
	}

	Status LoadSampleData(File& file, U64 dataStartOffset, U8* sampleBuffer, U32 blockSize, U32 writeOffset) noexcept
	{
		ZE_PERF_GUARD("Read WAV ASYNC+");
		ZE_ASSERT(sampleBuffer && blockSize, "Buffer must be allocated before loading sample data!");
		ZE_ASSERT(blockSize + writeOffset <= blockSize, "Reading beyond buffer bounds!");

		return file.Read(sampleBuffer + writeOffset, blockSize, dataStartOffset);
	}
}