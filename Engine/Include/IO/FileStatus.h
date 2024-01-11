#pragma once

namespace ZE::IO
{
	// Status returned by file operations
	enum class FileStatus : U8
	{
		Ok,
		ErrorOpeningFile,
		ErrorReading,
		ErrorWriting,
		ErrorBadSignature,
		ErrorUnknowVersion,
		ErrorNoResources,
		ErrorUnknownResourceEntry,
		ErrorMissingMaterialEntries,
		ErrorUnknownTextureSchema,
		ErrorEmptyTextureCount,
		ErrorIncorrectTextureEntry,
	};

	// Convert enum code to string representation for display
	constexpr const char* GetFileStatusString(FileStatus status) noexcept;

#pragma region Functions
	constexpr const char* GetFileStatusString(FileStatus status) noexcept
	{
		switch (status)
		{
		case FileStatus::Ok:
			return "OK";
		case FileStatus::ErrorReading:
			return "Error occured while reading the file";
		case FileStatus::ErrorOpeningFile:
			return "Error occured while opening the file";
		case FileStatus::ErrorBadSignature:
			return "File have incorrect signature";
		case FileStatus::ErrorUnknowVersion:
			return "Unknown version of the file";
		case FileStatus::ErrorNoResources:
			return "Resource pack with no resources";
		case FileStatus::ErrorUnknownResourceEntry:
			return "Unknown resource entry in resource pack";
		case FileStatus::ErrorMissingMaterialEntries:
			return "Material data incomplete, missing following Buffer and Texture entries or have they contain invalid data";
		case FileStatus::ErrorUnknownTextureSchema:
			return "Texture schema name is not recognized and not supported by this version of the engine";
		case FileStatus::ErrorEmptyTextureCount:
			return "Texture pack does not contain any textures";
		case FileStatus::ErrorIncorrectTextureEntry:
			return "Texture on given position does not match the expected texture type on this schema location or contains ill-formed data";
		default:
			return "UNKNOWN";
		}
	}
#pragma endregion
}