#pragma once

namespace ZE::GFX::API::VK
{
	// Information about single GPU allocation
	struct Allocation
	{
		enum class Usage : U8
		{
			// Fast memory for CPU, used for direct operations on Vulkan-visible buffers
			CPU,
			// Debug CPU memory, might be slower
			DebugCPU,
			// Memory written directly by GPU and can be efficiently read by CPU
			StagingToCPU,
			// Fast memory for GPU operations
			GPU,
			// Memory written directly by CPU and can be moderately fast read by GPU
			StagingToGPU,
			// Memory used in GPU-GPU transfer operations, no CPU access provided
			OnlyTransferGPU
		};

		AllocHandle Handle = 0;
		U32 MemoryIndex = UINT32_MAX;

		constexpr bool IsFree() const noexcept { return Handle == 0 && MemoryIndex == UINT32_MAX; }
		constexpr void Free() noexcept { Handle = 0; MemoryIndex = UINT32_MAX; }
	};
}