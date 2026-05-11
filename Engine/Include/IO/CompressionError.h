#pragma once
ZE_WARNING_PUSH
#include "bzlib.h"
ZE_WARNING_POP

namespace ZE::IO
{
	// Possible results of the compression operations
	enum class CompressionResult : S8
	{
		Bzip2_SequenceError = BZ_SEQUENCE_ERROR,
		Bzip2_ParamError = BZ_PARAM_ERROR,
		Bzip2_MemoryError = BZ_MEM_ERROR,
		Bzip2_DataError = BZ_DATA_ERROR,
		Bzip2_MagicError = BZ_DATA_ERROR_MAGIC,
		Bzip2_IOError = BZ_IO_ERROR,
		Bzip2_UnexpectedEOF = BZ_UNEXPECTED_EOF,
		Bzip2_OutputBuffFull = BZ_OUTBUFF_FULL,
		Bzip2_ConfigError = BZ_CONFIG_ERROR,
		Success = 0, Unknown, DecompressionSizeMismatch,
		ZLIB_InitCompressError, ZLIB_CompressError, ZLIB_EndCompressError,
		ZLIB_InitDecompressError, ZLIB_DecompressError, ZLIB_EndDecompressError,
	};

	// General compression codec error category
	class CompressionError : public std::error_category
	{
	protected:
		CompressionError() = default;

	public:
		ZE_CLASS_MOVE(CompressionError);
		virtual ~CompressionError() = default;

		static constexpr const std::error_category& GetCategory() noexcept { static CompressionError CATEGORY; return CATEGORY; }
		static Status Make(CompressionResult result) noexcept { return { static_cast<int>(result), GetCategory() }; }

		const char* name() const noexcept override { return "Compression Error"; }
		std::string message(int condition) const override;
	};
}

#define ZE_COMPRESS_ERROR(result) ZE::IO::CompressionError::Make(result)