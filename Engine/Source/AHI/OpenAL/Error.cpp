#include "AHI/OpenAL/Error.h"

namespace ZE::AHI::OpenAL
{
	std::string AlError::message(int condition) const
	{
		switch (static_cast<ALenum>(condition))
		{
		case AL_NO_ERROR:
			return "Success";
		case AL_INVALID_NAME:
			return "Invalid name (ID) parameter";
		case AL_INVALID_ENUM:
			return "Invalid enum parameter";
		case AL_INVALID_VALUE:
			return "Invalid value parameter";
		case AL_INVALID_OPERATION:
			return "Illegal call";
		case AL_OUT_OF_MEMORY:
			return "Not enough memory";
		default:
			return "Unknown";
		}
	}

	std::string AlcError::message(int condition) const
	{
		switch (static_cast<ALCenum>(condition))
		{
		case ALC_NO_ERROR:
			return "Success";
		case ALC_INVALID_DEVICE:
			return "Invalid device handle";
		case ALC_INVALID_CONTEXT:
			return "Invalid context handle";
		case ALC_INVALID_ENUM:
			return "Invalid enum parameter";
		case ALC_INVALID_VALUE:
			return "Invalid value parameter";
		case ALC_OUT_OF_MEMORY:
			return "Not enough memory";
		default:
			return "Unknown";
		}
	}
}