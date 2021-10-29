#pragma once
#include "GFX/API/DX11/Pipeline/FrameBuffer.h"
#include "GFX/API/DX12/Pipeline/FrameBuffer.h"

namespace ZE::GFX::Pipeline
{
	// Managing all writeable buffers used during single frame
	class FrameBuffer final
	{
		ZE_API_BACKEND(Pipeline::FrameBuffer) backend;

	public:
		FrameBuffer() = default;
		FrameBuffer(FrameBuffer&&) = delete;
		FrameBuffer(const FrameBuffer&) = delete;
		FrameBuffer& operator=(FrameBuffer&&) = delete;
		FrameBuffer& operator=(const FrameBuffer&) = delete;
		~FrameBuffer() = default;

		constexpr void Init(Device& dev, SwapChain& swapChain, FrameBufferDesc& desc) { backend.Init(dev, swapChain, desc); }
		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, SwapChain& swapChain) { /*backend.Switch(nextApi, dev, swapChain);*/ }
		constexpr ZE_API_BACKEND(Pipeline::FrameBuffer)& Get() noexcept { return backend; }
	};
}