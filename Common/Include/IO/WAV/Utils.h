#pragma once
#include "IO/File.h"
#include "SFX/AudioBuffer.h"
#include "ChunkHeaders.h"
#include "FileResult.h"

namespace ZE::IO::WAV
{
	// Identifier of WAV file 'RIFF'
	inline constexpr U32 RIFF_MAGIC_NUMBER = ZE_MAKE_FOURCC('R', 'I', 'F', 'F');
	// Identifier of WAV file format 'WAVE'
	inline constexpr U32 WAVE_MAGIC_NUMBER = ZE_MAKE_FOURCC('W', 'A', 'V', 'E');
	// Identifier of WAV file format chunk 'fmt '
	inline constexpr U32 FORMAT_MAGIC_NUMBER = ZE_MAKE_FOURCC('f', 'm', 't', ' ');
	// Identifier of WAV file chunk 'data'
	inline constexpr U32 DATA_MAGIC_NUMBER = ZE_MAKE_FOURCC('d', 'a', 't', 'a');

	// Load and parse WAV file information from disk. Returns file handle pointing at the start of the sample data
	template<bool ASYNC>
	Expected<SFX::AudioDesc> ParseFileInfo(File& file, U64 currentOffset = 0, U64* dataStartOffset = nullptr) noexcept;
	// Load sample data, must first call ParseFileInfo to get the buffer size and then allocate memory for it
	Status LoadSampleData(File& file, U8* sampleBuffer, U32 blockSize, U32 writeOffset) noexcept;

#pragma region Functions
	template<bool ASYNC>
	Expected<SFX::AudioDesc> ParseFileInfo(File& file, U64 currentOffset, U64* dataStartOffset) noexcept
	{
		auto checkRead = [&]<typename T>(T& item, U32 size) noexcept -> Status
		{
			Status status = {};
			if constexpr (ASYNC)
			{
				auto result = file.ReadAsync(&item, size, currentOffset);
				ZE_EXPECT_RET_FAILED_CODE(status, result.Get());
			}
			else
			{
				ZE_CODE_RET_FAILED(file.Read(&item, size));
			}
			currentOffset += sizeof(item);
			return status;
		};
#define ZE_WAV_CHECK_READ(item) ZE_CODE_RET_FAILED_EXPECT(checkRead(item, sizeof(item)))

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
			ZE_CODE_RET_FAILED_EXPECT(checkRead(formatChunk.FormatEx.ExtensionSize, sizeof(FormatExtensionChunkHeader) - sizeof(FormatChunkHeader)));
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
			if constexpr (!ASYNC)
			{
				ZE_CODE_RET_FAILED_EXPECT(file.SetOffset(currentOffset));
			}
			ZE_WAV_CHECK_READ(dataChunk);
		}

		if (dataStartOffset)
			*dataStartOffset = currentOffset;
		buffer.Bytes = dataChunk.SampleDataSize;
		return buffer;
#undef ZE_WAV_CHECK_READ
	}
#pragma endregion
}