#pragma once
#include "Types.h"

namespace ZE::GFX
{
	// Data for vertex buffer
	struct VertexData
	{
		U32 BufferSize;
		U32 VertexSize;
		U8* Vertices;
	};
}