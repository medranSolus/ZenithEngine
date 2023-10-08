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
		ErrorUnknownResourceEntry
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
		default:
			return "UNKNOWN";
		}
	}
#pragma endregion
}