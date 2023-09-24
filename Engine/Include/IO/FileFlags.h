#pragma once

namespace ZE::IO
{
	typedef U8 FileFlags;
	// How file would be open
	enum FileFlag : FileFlags
	{
		None = 0,
		// Enable specialized GPU access directly from disk if current API supports it
		GpuReading = 1,
		// Set this file as write destination, cannot be opened with other reading flags
		WriteOnly = 2
	};
}