#pragma once

namespace ZE::Data
{
	// Indicator in what currently is GPU resource
	enum class ResourceLocation : U8
	{
		GPU,
		Disk,
		Memory,
		DiskNoGPU,
		MemoryNoGPU,
		UploadingToGPU,
		PagingOutToMemory,
		PagingOutToDiskFromGPU,
		PagingOutToDiskFromMemory,
		PagingOutToDiskFromMemoryNoGPU,
	};

	// Thread-safe ResourceLocation
	typedef std::atomic<ResourceLocation> ResourceLocationAtom;
}