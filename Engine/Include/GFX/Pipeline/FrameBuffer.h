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

		constexpr void Init(Device& dev, CommandList& mainList, FrameBufferDesc& desc) { backend.Init(dev, mainList, desc); }
		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, SwapChain& swapChain) { /*backend.Switch(nextApi, dev, swapChain);*/ }
		constexpr ZE_API_BACKEND(Pipeline::FrameBuffer)& Get() noexcept { return backend; }

		// Main Gfx API

		constexpr void SwapBackbuffer(Device& dev, SwapChain& swapChain) { ZE_API_BACKEND_CALL(backend, SwapBackbuffer, dev, swapChain); }
		constexpr void InitTransitions(CommandList& cl) const noexcept { ZE_API_BACKEND_CALL(backend, InitTransitions, cl); }
		constexpr void EntryTransitions(U64 level, CommandList& cl) const noexcept { ZE_API_BACKEND_CALL(backend, EntryTransitions, level, cl); }
		constexpr void ExitTransitions(U64 level, CommandList& cl) const noexcept { ZE_API_BACKEND_CALL(backend, ExitTransitions, level, cl); }
	};
}