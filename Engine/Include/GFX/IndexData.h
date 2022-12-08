#pragma once
#include "Types.h"

namespace ZE::GFX
{
	// Data for index buffer
	struct IndexData
	{
		U32 Count;
		U8 IndexSize;
		const void* Indices;
	};
}