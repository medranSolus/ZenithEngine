#pragma once
#include "Pipeline/ResourceID.h"
#include "Resource/Generic.h"
#include "CommandList.h"
#include "FfxException.h"
ZE_WARNING_PUSH
#include "FidelityFX/host/ffx_types.h"
ZE_WARNING_POP

namespace ZE::GFX
{
	constexpr FfxDevice ffxGetDevice(Device& dev) noexcept { return (FfxDevice)&dev; }
	constexpr FfxCommandList ffxGetCommandList(CommandList& cl) noexcept { return (FfxCommandList)&cl; }
	constexpr FfxPipeline ffxGetPipeline(Resource::PipelineStateCompute& pipeline) noexcept { return (FfxPipeline)&pipeline; }

	FfxResource ffxGetResource(Pipeline::FrameBuffer& buffers, Resource::Generic& res, RID rid, Resource::State state) noexcept;
	void ffxGetInterface(Device& dev) noexcept;
}