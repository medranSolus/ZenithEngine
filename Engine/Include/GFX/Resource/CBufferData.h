#pragma once
#include "Data/Entity.h"
#include "IO/CompressionFormat.h"

namespace ZE::GFX::Resource
{
	// Data for cbuffer
	struct CBufferData
	{
		EID ResourceID = INVALID_EID;
		const void* Data = nullptr;
		U32 Bytes;
	};

	// Data for cbuffer from file buffer
	struct CBufferFileData
	{
		EID ResourceID = INVALID_EID;
		U64 BufferDataOffset = 0;
		U32 SourceBytes = 0;
		U32 UncompressedSize = 0;
		IO::CompressionFormat Compression = IO::CompressionFormat::None;
	};
}