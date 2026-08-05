#include "IO/WAV/Utils.h"
#include "IO/WAV/ChunkHeaders.h"
#include "IO/WAV/FileResult.h"

namespace ZE::IO::WAV
{
	Expected<SFX::AudioBuffer> ParseFile(File& file) noexcept
	{
#define ZE_WAV_CHECK_READ(item) ZE_CODE_RET_FAILED_EXPECT(file.Read(&item, sizeof(item))); currentOffset += sizeof(item)

		// Load all headers and check their validity
		U64 currentOffset = 0;
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
			ZE_CODE_RET_FAILED_EXPECT(file.Read(&formatChunk.FormatEx.ExtensionSize, sizeof(FormatExtensionChunkHeader) - sizeof(FormatChunkHeader)));
			currentOffset += sizeof(FormatExtensionChunkHeader) - sizeof(FormatChunkHeader);
			break;
		}
		default:
			return std::unexpected(Error::Make(FileResult::FormatChunkTooSmall));
		}

		// Parse audio description
		SFX::AudioBuffer buffer = {};
		buffer.SampleRate = formatChunk.FormatEx.Format.SampleRate;

		switch (formatChunk.FormatEx.ExtensionSize)
		{
		case Base(FormatExtensionHeaderSize::None):
		{
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
			file.SetOffset(currentOffset);
			ZE_WAV_CHECK_READ(dataChunk);
		}

		buffer.Bytes = dataChunk.SampleDataSize;
		if (buffer.Bytes)
		{
			buffer.Samples = std::make_shared<U8[]>(buffer.Bytes);
			ZE_CODE_RET_FAILED_EXPECT(file.Read(buffer.Samples.get(), buffer.Bytes));
		}
		return buffer;
#undef ZE_WAV_CHECK_READ
	}
}