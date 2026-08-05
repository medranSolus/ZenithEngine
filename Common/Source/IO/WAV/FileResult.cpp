#include "IO/WAV/FileResult.h"

namespace ZE::IO::WAV
{
	std::string Error::message(int condition) const
	{
		switch (static_cast<FileResult>(condition))
		{
		case FileResult::Ok:
			return "Ok";
		case FileResult::IncorrectMagicNumberRIFF:
			return "Incorrect magic number for RIFF chunk";
		case FileResult::IncorrectMagicNumberWAVE:
			return "Incorrect magic number for WAVE chunk";
		case FileResult::IncorrectMagicNumberFormatChunk:
			return "Incorrect magic number for format chunk";
		case FileResult::FileTooSmall:
			return "File is too small";
		case FileResult::FormatChunkTooSmall:
			return "Format chunk is too small";
		case FileResult::UnknownFormatChunkExtension:
			return "Unknown format chunk extension";
		case FileResult::IncorrectAudioFormat:
			return "Incorrect audio format";
		default:
			ZE_ENUM_UNHANDLED();
		case FileResult::Unknown:
			return "Unknown WAV error";
		}
	}
}