#pragma once
#if _ZE_RHI_DX11
#	include "RHI/DX11/Pipeline/FrameBuffer.h"
#endif
#if _ZE_RHI_DX12
#	include "RHI/DX12/Pipeline/FrameBuffer.h"
#endif
#if _ZE_RHI_VK
#	include "RHI/VK/Pipeline/FrameBuffer.h"
#endif

namespace ZE::GFX::Pipeline
{
	// Managing all writeable buffers used during single frame
	class FrameBuffer final
	{
		ZE_RHI_BACKEND(Pipeline::FrameBuffer);

	public:
		FrameBuffer() = default;
		ZE_CLASS_DELETE(FrameBuffer);
		~FrameBuffer() = default;

		//constexpr void Init(Device& dev, CommandList& mainList, FrameBufferDesc& desc) { ZE_RHI_BACKEND_VAR.Init(dev, mainList, desc); }
		//constexpr void SwitchApi(GfxApiType nextApi, Device& dev, CommandList& mainList) { /*ZE_RHI_BACKEND_VAR.Switch(nextApi, dev, mainList);*/ }
		ZE_RHI_BACKEND_GET(Pipeline::FrameBuffer);

		// Main Gfx API
	};
}