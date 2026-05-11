#include "IO/CompressionError.h"

namespace ZE::IO
{
	std::string CompressionError::message(int condition) const
	{
		switch (static_cast<CompressionResult>(condition))
		{
		default:
			ZE_ENUM_UNHANDLED();
		case CompressionResult::Unknown:
			return "Unknown error";
		case CompressionResult::Success:
			return "Success";
		case CompressionResult::DecompressionSizeMismatch:
			return "Mismatch between expected and real decompression size";
		case CompressionResult::ZLIB_InitCompressError:
			return "Error initializing ZLIB deflate compression";
		case CompressionResult::ZLIB_CompressError:
			return "Error performing ZLIB deflate compression";
		case CompressionResult::ZLIB_EndCompressError:
			return "Error ending ZLIB deflate compression";
		case CompressionResult::ZLIB_InitDecompressError:
			return "Error initializing ZLIB inflate decompression";
		case CompressionResult::ZLIB_DecompressError:
			return "Error performing ZLIB inflate decompression";
		case CompressionResult::ZLIB_EndDecompressError:
			return "Error ending ZLIB inflate decompression";
		case CompressionResult::Bzip2_SequenceError:
			return "Bzip2 sequence error";
		case CompressionResult::Bzip2_ParamError:
			return "Bzip2 parameter error";
		case CompressionResult::Bzip2_MemoryError:
			return "Bzip2 memory error";
		case CompressionResult::Bzip2_DataError:
			return "Bzip2 data error";
		case CompressionResult::Bzip2_MagicError:
			return "Bzip2 data magic number error";
		case CompressionResult::Bzip2_IOError:
			return "Bzip2 IO error";
		case CompressionResult::Bzip2_UnexpectedEOF:
			return "Bzip2 unexpected end of file error";
		case CompressionResult::Bzip2_OutputBuffFull:
			return "Bzip2 output buffer full error";
		case CompressionResult::Bzip2_ConfigError:
			return "Bzip2 config error";
		}
	}
}