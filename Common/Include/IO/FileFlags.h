#pragma once
#include "BasicTypes.h"

namespace ZE::IO
{
	// Flags controlling opening of files
	typedef U8 FileFlags;

	// Possible modes of opening files
	enum class FileFlag : FileFlags
	{
		ReadMode = 1, // If no mode is specified, defaults to read mode
		WriteMode = 2,
		SequentialAccess = 4, // Indicate that access to a file will be sequential (impacts caching strategy of OS)
		RandomAccess = 8, // Indicate that access to a file will be random (impacts caching strategy of OS)
		WriteThrough = 16, // Skip some of the buffering provided by OS
		CreateOnOpen = 32, // Create file if not exist
		TruncateOnOpen = 64, // When opening file, truncate it's content. If write mode is not specified then this flag is ignored
		EnableAsync = 128, // Enables async operations (can result in slower synchronous operations)
		Default = RandomAccess | WriteThrough, // Default strategy for file access that provides most performance in typical usage
		DefaultRead = Default | ReadMode,
		DefaultWrite = Default | WriteMode | CreateOnOpen | TruncateOnOpen
	};
	ZE_ENUM_OPERATORS(FileFlag, FileFlags);
}