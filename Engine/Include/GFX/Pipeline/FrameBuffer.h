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
		constexpr void InitRTV(CommandList& cl, RID rid) const noexcept { ZE_API_BACKEND_CALL(InitRTV, cl, rid); }
		// Depth stencil before first use must be initialized or cleared
		constexpr void InitDSV(CommandList& cl, RID rid) const noexcept { ZE_API_BACKEND_CALL(InitDSV, cl, rid); }

		// Maybe add also ability to set scale and offset for viewport if needed
		constexpr void SetRTV(CommandList& cl, RID rid) const { ZE_API_BACKEND_CALL(SetRTV, cl, rid); }
		constexpr void SetDSV(CommandList& cl, RID rid) const { ZE_API_BACKEND_CALL(SetDSV, cl, rid); }
		constexpr void SetOutput(CommandList& cl, RID rtv, RID dsv) const { ZE_API_BACKEND_CALL(SetOutput, cl, rtv, dsv); }

		template<U32 RTVCount>
		constexpr void SetRTV(CommandList& cl, const RID* rid) const { ZE_API_BACKEND_CALL(SetRTV<RTVCount>, cl, rid); }
		template<U32 RTVCount>
		constexpr void SetOutput(CommandList& cl, const RID* rtv, RID dsv) const { ZE_API_BACKEND_CALL(SetOutput<RTVCount>, cl, rtv, dsv); }

		// When current bind slot is inside BufferPack then only one call for first resource is required in case of resource adjacency.
		// Resources are considered adjacent when during creation in FrameBufferDesc they have been specified one by one.
		// In case of another resource between them that is not used as SRV or UAV then such resources are still adjacent
		constexpr void SetSRV(CommandList& cl, Binding::Context& bindCtx, RID rid) const { ZE_API_BACKEND_CALL(SetSRV, cl, bindCtx, rid); }
		// When current bind slot is inside BufferPack then only one call for first resource is required in case of resource adjacency.
		// Resources are considered adjacent when during creation in FrameBufferDesc they have been specified one by one.
		// In case of another resource between them that is not used as SRV or UAV then such resources are still adjacent
		constexpr void SetUAV(CommandList& cl, Binding::Context& bindCtx, RID rid) const { ZE_API_BACKEND_CALL(SetUAV, cl, bindCtx, rid); }

		// Render target before first use must be initialized or cleared (except backbuffer)
		constexpr void ClearRTV(CommandList& cl, RID rid, const ColorF4& color) const { ZE_API_BACKEND_CALL(ClearRTV, cl, rid, color); }
		// Depth stencil before first use must be initialized or cleared
		constexpr void ClearDSV(CommandList& cl, RID rid, float depth, U8 stencil) const { ZE_API_BACKEND_CALL(ClearDSV, cl, rid, depth, stencil); }
		constexpr void ClearUAV(CommandList& cl, RID rid, const ColorF4& color) const { ZE_API_BACKEND_CALL(ClearUAV, cl, rid, color); }
		constexpr void ClearUAV(CommandList& cl, RID rid, const Pixel colors[4]) const { ZE_API_BACKEND_CALL(ClearUAV, cl, rid, colors); }

		constexpr void SwapBackbuffer(Device& dev, SwapChain& swapChain) { ZE_API_BACKEND_CALL(SwapBackbuffer, dev, swapChain); }
		constexpr void InitTransitions(Device& dev, CommandList& cl) const { ZE_API_BACKEND_CALL(InitTransitions, dev, cl); }
		constexpr void ExitTransitions(Device& dev, CommandList& cl, U64 level) const noexcept { ZE_API_BACKEND_CALL(ExitTransitions, dev, cl, level); }
	};
}