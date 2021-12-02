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
		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, CommandList& mainList) { /*backend.Switch(nextApi, dev, mainList);*/ }
		constexpr ZE_API_BACKEND(Pipeline::FrameBuffer)& Get() noexcept { return backend; }

		// Main Gfx API

		// Render target before first use must be initialized or cleared (except backbuffer)
		constexpr void InitRTV(GFX::CommandList& cl, U64 rid) const noexcept { ZE_API_BACKEND_CALL(backend, InitRTV, cl, rid); }
		// Depth stencil before first use must be initialized or cleared
		constexpr void InitDSV(GFX::CommandList& cl, U64 rid) const noexcept { ZE_API_BACKEND_CALL(backend, InitDSV, cl, rid); }

		// Render target before first use must be initialized or cleared (except backbuffer)
		constexpr void ClearRTV(GFX::Device& dev, GFX::CommandList& cl, U64 rid, const ColorF4 color) const { ZE_API_BACKEND_CALL(backend, ClearRTV, dev, cl, rid, color); }
		// Depth stencil before first use must be initialized or cleared
		constexpr void ClearDSV(GFX::Device& dev, GFX::CommandList& cl, U64 rid, float depth, U8 stencil) const { ZE_API_BACKEND_CALL(backend, ClearDSV, dev, cl, rid, depth, stencil); }

		constexpr void SwapBackbuffer(Device& dev, SwapChain& swapChain) { ZE_API_BACKEND_CALL(backend, SwapBackbuffer, dev, swapChain); }
		constexpr void InitTransitions(Device& dev, CommandList& cl) const { ZE_API_BACKEND_CALL(backend, InitTransitions, dev, cl); }
		constexpr void ExitTransitions(Device& dev, CommandList& cl, U64 level) const noexcept { ZE_API_BACKEND_CALL(backend, ExitTransitions, dev, cl, level); }
	};
}