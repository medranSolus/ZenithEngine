#pragma once
#include "Types.h"

namespace ZE::GFX
{
	struct VertexData
	{
		U32 BufferSize;
		U32 VertexSize;
		U8* Vertices;
	};
}