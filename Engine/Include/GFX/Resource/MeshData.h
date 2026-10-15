#pragma once
#include "IO/CompressionFormat.h"

namespace ZE::GFX::Resource
{
	// Set of options to create mesh
	typedef U8 MeshOptions;
	// Possible options when creating mesh
	enum class MeshOption : MeshOptions
	{
		None = 0,
		// It is required that at the begining of the vertex data there should be Float4 aligned bounding box data (32 bytes)
		SourceDataFormatBBQ = 1,
		IndexFormat8bit = 2,
		IndexFormat16bit = 4,
	};
	ZE_ENUM_OPERATORS(MeshOption, MeshOptions);

	// Geometry data for mesh
	struct MeshData
	{
		static constexpr U32 VERTEX_BUFFER_ALIGNMENT = 4;

		// Packed vertex and index data in tight order: indices + vertices (aligned to `MeshData::VERTEX_BUFFER_ALIGNMENT`)
		std::shared_ptr<U8[]> PackedMesh = nullptr;
		U32 VertexCount = 0;
		U32 IndexCount = 0;
		U16 VertexSize = 0;
		MeshOptions Options = Base(MeshOption::None);
	};

	// Geometry data for mesh from file buffer
	struct MeshFileData
	{
		U64 MeshDataOffset = 0;
		U32 VertexCount = 0;
		U32 IndexCount = 0;
		U32 SourceBytes = 0;
		U16 VertexSize = 0;
		MeshOptions Options = Base(MeshOption::None);
		IO::CompressionFormat Compression = IO::CompressionFormat::None;
	};
}