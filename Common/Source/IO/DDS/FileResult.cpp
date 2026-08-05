#include "IO/DDS/FileResult.h"

namespace ZE::IO::DDS
{
	std::string Error::message(int condition) const
	{
		switch (static_cast<FileResult>(condition))
		{
		case FileResult::Ok:
			return "Ok";
		case FileResult::IncorrectMagicNumber:
			return "Incorrect DDS magic number";
		case FileResult::UnknownFormat:
			return "Not supported pixel format";
		case FileResult::MissingCubemapFaces:
			return "Not all cubemap faces defined";
		case FileResult::IllformattedVolumeTexture:
			return "Incorrectly formatted volume texture (ex. array size bigger than 1)";
		case FileResult::IncorrectArraySize:
			return "Wrong value used as array size";
		case FileResult::Incorrect1DTextureHeight:
			return "1D texture with height different than 1";
		case FileResult::IncorrectDimension:
			return "Unknown texture dimension";
		default:
			ZE_ENUM_UNHANDLED();
		case FileResult::Unknown:
			return "Unknown DDS error";
		}
	}
}