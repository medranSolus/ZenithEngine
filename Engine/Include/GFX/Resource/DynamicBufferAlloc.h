#pragma once

namespace ZE::GFX::Resource
{
	// Info about allocated memory inside dynamic buffer
	struct DynamicBufferAlloc
	{
		U32 Offset;
		U64 Block;
	};
}