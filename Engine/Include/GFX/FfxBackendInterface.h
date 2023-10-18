#pragma once
#include "Pipeline/ResourceID.h"
#include "Resource/DynamicCBuffer.h"
#include "Resource/Generic.h"
#include "CommandList.h"
#include "ChainPool.h"
#include "FfxException.h"
ZE_WARNING_PUSH
#include "FidelityFX/host/ffx_types.h"
ZE_WARNING_POP

namespace ZE::GFX
{
	// Convert command list into handle used by FFX SDK
	constexpr FfxCommandList ffxGetCommandList(CommandList& cl) noexcept { return (FfxCommandList)&cl; }

	// Initialize proxy Generic resource from one of the frame buffers and create handle for FFX SDK (created resource will be freed internally)
	FfxResource ffxGetResource(Pipeline::FrameBuffer& buffers, Resource::Generic& res, RID rid, Resource::State state) noexcept;

	// Fill up pointers to FFX SDK backend callbacks
	void ffxGetInterface(Device& dev, ChainPool<Resource::DynamicCBuffer>& dynamicBuffers) noexcept;

	// Free up FFX SDK backend interface
	void ffxDestroyInterface(Device& dev) noexcept;
}