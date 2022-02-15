#pragma once
#include "Types.h"

namespace ZE::GFX
{
	// Data for vertex buffer
	struct VertexData
	{
		U32 Count;
		U32 VertexSize;
		const void* Vertices;
	};
}