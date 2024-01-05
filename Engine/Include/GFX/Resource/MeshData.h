#pragma once
#include "Data/Entity.h"
#include "IO/CompressionFormat.h"

namespace ZE::GFX::Resource
{
	// Geometry data for mesh
	struct MeshData
	{
		static constexpr U32 VERTEX_BUFFER_ALIGNMENT = 4;

		EID MeshID = INVALID_EID;
		// Packed vertex and index data in tight order: indices + vertices (aligned to `MeshData::VERTEX_BUFFER_ALIGNMENT`)
		std::shared_ptr<U8[]> PackedMesh = nullptr;
		U32 VertexCount = 0;
		U32 IndexCount = 0;
		U16 VertexSize = 0;
		U8 IndexSize = 0;
	};

	// Geometry data for mesh from file buffer
	struct MeshFileData
	{
		EID MeshID = INVALID_EID;
		U64 MeshDataOffset = 0;
		U32 VertexCount = 0;
		U32 IndexCount = 0;
		U32 SourceBytes = 0;
		U32 UncompressedSize = 0;
		U16 VertexSize = 0;
		PixelFormat IndexFormat = PixelFormat::Unknown;
		IO::CompressionFormat Compression = IO::CompressionFormat::None;
	};
}