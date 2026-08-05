#pragma once
#include "BasicTypes.h"
#include <string>

namespace ZE::IO::WAV
{
	// Result of WAV file operations
	enum class FileResult : U8
	{
		Ok,
		IncorrectMagicNumberRIFF,
		IncorrectMagicNumberWAVE,
		IncorrectMagicNumberFormatChunk,
		FileTooSmall,
		FormatChunkTooSmall,
		UnknownFormatChunkExtension,
		IncorrectAudioFormat,
		Unknown
	};

	// Main handler of WAV related errors
	class Error : public std::error_category
	{
	protected:
		Error() = default;

	public:
		ZE_CLASS_MOVE(Error);
		virtual ~Error() = default;

		static constexpr const std::error_category& GetCategory() noexcept { static Error CATEGORY; return CATEGORY; }
		static Status Make(FileResult result) noexcept { return { static_cast<int>(result), GetCategory() }; }

		const char* name() const noexcept override { return "WAV Error"; }
		std::string message(int condition) const override;
	};
}