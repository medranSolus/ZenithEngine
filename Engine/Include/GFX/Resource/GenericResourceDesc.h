#pragma once
#include "State.h"

namespace ZE::GFX::Resource
{
	// Type of Generic resource (how it will be created on GPU
	enum class GenericResourceType : U8
	{
		Buffer, Texture1D, Texture2D, TextureCube, Texture3D
	};

	// Location where to create Generic resource
	enum class GenericResourceHeap : U8
	{
		GPU, Upload, Readback
	};

	typedef U8 GenericResourceFlags;
	// Flags for creation of Generic resource
	enum GenericResourceFlag : GenericResourceFlags
	{
		ReadOnly = 0, // Have to provide init data
		RenderTarget = 1, // Can't be used together with DepthBuffer
		UnorderedAccess = 2,
		DepthBuffer = 4, // Can't be used together with RenderTarget
		IndirectArguments = 8,
		ArrayView = 16
	};

	// Creation descriptor for Generic GPU resource
	struct GenericResourceDesc
	{
		GenericResourceType Type;
		GenericResourceHeap Heap;
		GenericResourceFlags Flags;
		PixelFormat Format;
		U16 MipCount;
		U32 WidthOrBufferSize;
		U32 HeightOrBufferStride;
		U16 DepthOrArraySize;
		GFX::Resource::State InitState;
		// Not allowed for Readback heaps
		U32 InitDataSize;
		void* InitData;
#if _ZE_DEBUG_GFX_NAMES
		std::string DebugName = "Unknown";
#endif
	};
}

#if _ZE_DEBUG_GFX_NAMES
// Sets name to be used as indentificator of created Generic resource
#	define ZE_GEN_RES_SET_NAME(resDesc, name) resDesc.DebugName = name
#else
// Sets name to be used as indentificator of created Generic resource
#	define ZE_GEN_RES_SET_NAME(resDesc, name)
#endif