#include "IO/ResourcePackError.h"

namespace ZE::IO
{
	std::string ResourcePackError::message(int condition) const
	{
		switch (static_cast<ResourcePackStatus>(condition))
		{
		default:
			ZE_ENUM_UNHANDLED();
		case ResourcePackStatus::ErrorUnkown:
			return "Unknown error";
		case ResourcePackStatus::Ok:
			return "OK";
		case ResourcePackStatus::ErrorBadSignature:
			return "File have incorrect signature";
		case ResourcePackStatus::ErrorUnknowVersion:
			return "Unknown version of the file";
		case ResourcePackStatus::ErrorNoResources:
			return "Resource pack with no resources";
		case ResourcePackStatus::ErrorUnknownResourceEntry:
			return "Unknown resource entry in resource pack";
		case ResourcePackStatus::ErrorMissingMaterialEntries:
			return "Material data incomplete, missing following Buffer and Texture entries or have they contain invalid data";
		case ResourcePackStatus::ErrorUnknownTextureSchema:
			return "Texture schema name is not recognized and not supported by this version of the engine";
		case ResourcePackStatus::ErrorEmptyTextureCount:
			return "Texture pack does not contain any textures";
		case ResourcePackStatus::ErrorIncorrectTextureEntry:
			return "Texture on given position does not match the expected texture type on this schema location or contains ill-formed data";
		case ResourcePackStatus::ErrorIncorrectMaterialBufferSize:
			return "Material data does not match expected size of the material buffer";
		}
	}
}