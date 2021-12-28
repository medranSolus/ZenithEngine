#pragma once
#include "GFX/API/DX11/Pipeline/FrameBuffer.h"
#include "GFX/API/DX12/Pipeline/FrameBuffer.h"

namespace ZE::GFX::Pipeline
{
	// Managing all writeable buffers used during single frame
	class FrameBuffer final
	{
		ZE_API_BACKEND(Pipeline::FrameBuffer);

	public:
		FrameBuffer() = default;
		ZE_CLASS_DELETE(FrameBuffer);
		~FrameBuffer() = default;

		constexpr void Init(Device& dev, CommandList& mainList, FrameBufferDesc& desc) { ZE_API_BACKEND_VAR.Init(dev, mainList, desc); }
		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, CommandList& mainList) { /*ZE_API_BACKEND_VAR.Switch(nextApi, dev, mainList);*/ }
		ZE_API_BACKEND_GET(Pipeline::FrameBuffer);

		// Main Gfx API

		// Render target before first use must be initialized or cleared (except backbuffer)
		constexpr void InitRTV(GFX::CommandList& cl, RID rid) const noexcept { ZE_API_BACKEND_CALL(InitRTV, cl, rid); }
		// Depth stencil before first use must be initialized or cleared
		constexpr void InitDSV(GFX::CommandList& cl, RID rid) const noexcept { ZE_API_BACKEND_CALL(InitDSV, cl, rid); }

		constexpr void SetRTV(GFX::Device& dev, GFX::CommandList& cl, RID rid) const { ZE_API_BACKEND_CALL(SetRTV, dev, cl, rid); }
		constexpr void SetDSV(GFX::Device& dev, GFX::CommandList& cl, RID rid) const { ZE_API_BACKEND_CALL(SetDSV, dev, cl, rid); }
		constexpr void SetOutput(GFX::Device& dev, GFX::CommandList& cl, RID rtv, RID dsv) const { ZE_API_BACKEND_CALL(SetOutput, dev, cl, rtv, dsv); }

		template<U32 RTVCount>
		constexpr void SetRTV(GFX::Device& dev, GFX::CommandList& cl, const RID* rid) const { ZE_API_BACKEND_CALL(SetRTV<RTVCount>, dev, cl, rid); }
		template<U32 RTVCount>
		constexpr void SetOutput(GFX::Device& dev, GFX::CommandList& cl, const RID* rtv, RID dsv) const { ZE_API_BACKEND_CALL(SetOutput<RTVCount>, dev, cl, rtv, dsv); }

		// Render target before first use must be initialized or cleared (except backbuffer)
		constexpr void ClearRTV(GFX::Device& dev, GFX::CommandList& cl, RID rid, const ColorF4 color) const { ZE_API_BACKEND_CALL(ClearRTV, dev, cl, rid, color); }
		// Depth stencil before first use must be initialized or cleared
		constexpr void ClearDSV(GFX::Device& dev, GFX::CommandList& cl, RID rid, float depth, U8 stencil) const { ZE_API_BACKEND_CALL(ClearDSV, dev, cl, rid, depth, stencil); }

		constexpr void SwapBackbuffer(Device& dev, SwapChain& swapChain) { ZE_API_BACKEND_CALL(SwapBackbuffer, dev, swapChain); }
		constexpr void InitTransitions(Device& dev, CommandList& cl) const { ZE_API_BACKEND_CALL(InitTransitions, dev, cl); }
		constexpr void ExitTransitions(Device& dev, CommandList& cl, U64 level) const noexcept { ZE_API_BACKEND_CALL(ExitTransitions, dev, cl, level); }
	};
}