#include "SFX/FileError.h"
ZE_WARNING_PUSH
#include "vorbis/codec.h"
#include "opusfile.h"
ZE_WARNING_POP

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

	std::string VorbisError::message(int condition) const
	{
		switch (condition)
		{
		default:
			return "Unknown";
		case 0:
			return "Ok";
		case OV_FALSE:
			return "False";
		case OV_EOF:
			return "EOF";
		case OV_HOLE:
			return "Skipping missing or corrupt data";
		case OV_EREAD:
			return "Error fetching compressed data";
		case OV_EFAULT:
			return "Internal codec fatal error";
		case OV_EIMPL:
			return "Feature not implemented";
		case OV_EINVAL:
			return "Invalid argument";
		case OV_ENOTVORBIS:
			return "Not Vorbis stream";
		case OV_EBADHEADER:
			return "Corrupted Vorbis header";
		case OV_EVERSION:
			return "Not supported format version";
		case OV_ENOTAUDIO:
			return "Not an audio packet";
		case OV_EBADPACKET:
			return "Corrupted packet";
		case OV_EBADLINK:
			return "Invalid link";
		case OV_ENOSEEK:
			return "Cannot seak stream";
		}
	}

	std::string OpusError::message(int condition) const
	{
		switch (condition)
		{
		default:
			return "Unknown";
		case 0:
			return "Ok";
		case OP_FALSE:
			return "False";
		case OP_EOF:
			return "EOF";
		case OP_HOLE:
			return "Hole in page sequence (corrupt or missing)";
		case OP_EREAD:
			return "Error fetching compressed data";
		case OP_EFAULT:
			return "Internal codec fatal error";
		case OP_EIMPL:
			return "Feature not implemented";
		case OP_EINVAL:
			return "Invalid argument";
		case OP_ENOTFORMAT:
			return "Not Opus stream";
		case OP_EBADHEADER:
			return "Corrupted Opus header";
		case OP_EVERSION:
			return "Not supported format version";
		case OP_ENOTAUDIO:
			return "Not an audio packet";
		case OP_EBADPACKET:
			return "Failed to decode packet";
		case OP_EBADLINK:
			return "Invalid link";
		case OP_ENOSEEK:
			return "Cannot seak stream";
		case OP_EBADTIMESTAMP:
			return "Link failed position validation";
		}
	}
}