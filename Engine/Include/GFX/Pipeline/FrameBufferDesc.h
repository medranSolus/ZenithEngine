#pragma once
#include "FrameResourceDesc.h"

namespace ZE::GFX::Pipeline
{
	// Flags controlling creation of FrameBuffer
	typedef U8 FrameBufferFlags;

	// Possible modes of crating FrameBuffer
	enum class FrameBufferFlag : FrameBufferFlags
	{
		None = 0x00,
		// Disable packing of FrameBuffer memory and aliasing it between resources (disables all creation algorithms)
		NoMemoryAliasing = 0x01,
		// Enable printing of FrameBuffer memory allocation for supported APIs (not possible on release builds)
		DebugMemoryPrint = 0x02,
		// Use matrix-based algorithm for creation of FrameBuffer in DX12
		MatrixCreationAlgorithmDX12 = 0x04,
		FavorCreationSpeed = MatrixCreationAlgorithmDX12,
	};

	ZE_ENUM_OPERATORS(FrameBufferFlag, FrameBufferFlags);

	// FrameBuffer creation data
	struct FrameBufferDesc
	{
		FrameBufferFlags Flags;
		std::vector<FrameResourceDesc> Resources;
		U32 PassLevelCount;
		// Start | End level
		std::vector<std::pair<U32, U32>> ResourceLifetimes;
	};
}