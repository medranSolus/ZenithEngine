#pragma once
#include "BasicTypes.h"
#include <string>
ZE_WARNING_PUSH
#include "FLAC/stream_decoder.h"
ZE_WARNING_POP

namespace ZE::SFX::FileError
{
	// Main handler of FLAC decoder errors related to initialization
	class FlacDecoderInit : public std::error_category
	{
	protected:
		FlacDecoderInit() = default;

	public:
		ZE_CLASS_MOVE(FlacDecoderInit);
		virtual ~FlacDecoderInit() = default;

		static constexpr const std::error_category& GetCategory() noexcept { static FlacDecoderInit CATEGORY; return CATEGORY; }
		static Status Make(FLAC__StreamDecoderInitStatus result) noexcept { return { static_cast<int>(result), GetCategory() }; }

		const char* name() const noexcept override { return "FLAC Decoder Init Error"; }
		std::string message(int condition) const override;
	};

	// Main handler of FLAC decoder related errors
	class FlacDecoderError : public std::error_category
	{
	protected:
		FlacDecoderError() = default;

	public:
		ZE_CLASS_MOVE(FlacDecoderError);
		virtual ~FlacDecoderError() = default;

		static constexpr const std::error_category& GetCategory() noexcept { static FlacDecoderError CATEGORY; return CATEGORY; }
		static Status Make(FLAC__StreamDecoderErrorStatus result) noexcept { return { static_cast<int>(result), GetCategory() }; }

		const char* name() const noexcept override { return "FLAC Decoder Error"; }
		std::string message(int condition) const override;
	};
}

#define ZE_FLAC_DECODER_INIT_ERROR(result) ZE::SFX::FileError::FlacDecoderInit::Make(result)
#define ZE_FLAC_DECODER_ERROR(result) ZE::SFX::FileError::FlacDecoderError::Make(result)