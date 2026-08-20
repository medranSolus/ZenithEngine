#include "SFX/FileError.h"

namespace ZE::SFX::FileError
{
	std::string FlacDecoderInit::message(int condition) const
	{
		if (condition >= 0 && condition <= FLAC__STREAM_DECODER_INIT_STATUS_ALREADY_INITIALIZED)
			return FLAC__StreamDecoderInitStatusString[condition];
		return "Unknown";
	}

	std::string FlacDecoderError::message(int condition) const
	{
		if (condition >= 0 && condition <= FLAC__STREAM_DECODER_ERROR_STATUS_MISSING_FRAME)
			return FLAC__StreamDecoderErrorStatusString[condition];
		return "Unknown";
	}
}