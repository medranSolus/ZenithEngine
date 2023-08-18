#pragma once
#include "Types.h"

namespace ZE::GFX
{
	// Geometry data for mesh
	struct MeshData
	{
		const void* Vertices = nullptr;
		const void* Indices = nullptr;
		U32 VertexCount = 0;
		U32 IndexCount = 0;
		U16 VertexSize = 0;
		U8 IndexSize = 0;
	};
}