#pragma once
#include "Types.h"

namespace ZE::GFX::Resource
{
	// Possible resource states
	enum State : U32
	{
		// State not specified, common state
		StateCommon = 0,
		// Frame to present on screen
		StatePresent = 0,

		StateVertexBuffer = 0x01, // Read-only
		StateConstantBuffer = 0x02, // Read-only
		StateIndexBuffer = 0x04, // Read-only
		StateShaderResourceNonPS = 0x08, // Read-only
		StateShaderResourcePS = 0x10, // Read-only
		StateShaderResourceAll = StateShaderResourceNonPS | StateShaderResourcePS, // Read-only
		StateCopySource = 0x20, // Read-only
		StateIndirect = 0x40, // Read-only
		StateDepthRead = 0x80, // Read-only
		// Allow reads as any resource, required for upload heap
		StateGenericRead = StateVertexBuffer | StateConstantBuffer
			| StateIndexBuffer | StateShaderResourceAll
			| StateCopySource | StateIndirect,

		StateRenderTarget = 0x100, // Write-only
		StateCopyDestination = 0x200, // Write-only
		StateStreamOut = 0x400, // Write-only
		StateDepthWrite = 0x800, // Write-only

		StateUnorderedAccess = 0x1000, // Read-write
		StateAccelerationStructureRT = 0x2000,
		StateShadingRateSource = 0x4000,
		StatePredication = 0x8000,

		StateResolveDestination = 0x10000,
		StateResolveSource = 0x20000,

		StateVideoDecodeRead = 0x100000,
		StateVideoProcessRead = 0x200000,
		StateVideoEncodeRead = 0x400000,

		StateVideoDecodeWrite = 0x100000,
		StateVideoProcessWrite = 0x2000000,
		StateVideoEncodeWrite = 0x4000000,
	};

	// Check if resource is in supported graphics read state
	constexpr bool IsReadOnlyState(State state) noexcept { return state & 0xFF; }
	// Check if resource is in supported graphics write state
	constexpr bool IsWriteEnabledState(State state) noexcept { return state & 0x1F00; }

	constexpr State& operator|=(State& s1, State s2) noexcept { return s1 = static_cast<State>(s1 | s2); }
	constexpr State& operator&=(State& s1, State s2) noexcept { return s1 = static_cast<State>(s1 & s2); }
}