#pragma once

namespace ZE::GFX::API::VK
{
	// Information about single GPU allocation
	struct Allocation
	{
		enum class Usage : U8 { CPU, GPU, StagingToCPU, StagingToGPU, DebugCPU, ProtectedGPU };

		AllocHandle Handle = 0;
		U32 MemoryIndex = UINT32_MAX;

		constexpr bool IsFree() const noexcept { return Handle == 0 && MemoryIndex == UINT32_MAX; }
		constexpr void Free() noexcept { Handle = 0; MemoryIndex = UINT32_MAX; }
	};
}