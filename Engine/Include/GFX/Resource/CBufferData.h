#pragma once
#include "IO/CompressionFormat.h"

namespace ZE::GFX::Resource
{
	// Data for cbuffer
	struct CBufferData
	{
		// Use this data source when original buffer lifetime is longer than upload time
		const void* DataStatic = nullptr;
		// Use this data source when original buffer lifetime ends before waiting for upload to finish
		std::shared_ptr<U8[]> DataRef = nullptr;
		U32 Bytes = 0;
	};

	// Data for cbuffer from file buffer
	struct CBufferFileData
	{
		U64 BufferDataOffset = 0;
		U32 SourceBytes = 0;
		U32 UncompressedSize = 0;
		IO::CompressionFormat Compression = IO::CompressionFormat::None;
	};
}