#pragma once
#include "GFX/API/DX11/Pipeline/FrameBuffer.h"
#include "GFX/API/DX12/Pipeline/FrameBuffer.h"
#include "GFX/API/VK/Pipeline/FrameBuffer.h"

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

		// Get width and height of the resource
		constexpr UInt2 GetDimmensions(RID rid) const noexcept { UInt2 dimm; ZE_API_BACKEND_CALL_RET(dimm, GetDimmensions, rid); return dimm; }

		// Render target before first use must be initialized, cleared or overwritten by full resource copy (except backbuffer)
		constexpr void InitRTV(CommandList& cl, RID rid) const noexcept { ZE_API_BACKEND_CALL(InitRTV, cl, rid); }
		// Depth stencil before first use must be initialized, cleared or overwritten by full resource copy
		constexpr void InitDSV(CommandList& cl, RID rid) const noexcept { ZE_API_BACKEND_CALL(InitDSV, cl, rid); }

		// Render targets (except backbuffer) and Depth stencils before first use must be initialized, cleared or overwritten by full resource copy
		constexpr void Copy(CommandList& cl, RID src, RID dest) const noexcept { ZE_API_BACKEND_CALL(Copy, cl, src, dest); }

		// Maybe add also ability to set scale and offset for viewport if needed
		constexpr void SetRTV(CommandList& cl, RID rid) const noexcept { ZE_API_BACKEND_CALL(SetRTV, cl, rid); }
		constexpr void SetRTV(CommandList& cl, RID rid, U16 mipLevel) const noexcept { ZE_API_BACKEND_CALL(SetRTV, cl, rid, mipLevel); }
		constexpr void SetDSV(CommandList& cl, RID rid) const noexcept { ZE_API_BACKEND_CALL(SetDSV, cl, rid); }
		constexpr void SetDSV(CommandList& cl, RID rid, U16 mipLevel) const noexcept { ZE_API_BACKEND_CALL(SetDSV, cl, rid, mipLevel); }
		constexpr void SetOutput(CommandList& cl, RID rtv, RID dsv) const noexcept { ZE_API_BACKEND_CALL(SetOutput, cl, rtv, dsv); }

		// When render targets have been created one after one without any depth stencil between them
		// they are considered adjacent which can speed-up their setting in the pipeline.
		// WARNING! Resources with higher mips levels are never adjacent with resources created after them!
		template<U32 RTVCount>
		constexpr void SetRTV(CommandList& cl, const RID* rid, bool adjacent = false) const noexcept { ZE_API_BACKEND_CALL(SetRTV<RTVCount>, cl, rid, adjacent); }
		// When render targets have been created one after one without any depth stencil between them
		// they are considered adjacent which can speed-up their setting in the pipeline.
		// WARNING! Resources with higher mips levels are never adjacent with resources created after them!
		template<U32 RTVCount>
		constexpr void SetOutput(CommandList& cl, const RID* rtv, RID dsv, bool adjacent = false) const noexcept { ZE_API_BACKEND_CALL(SetOutput<RTVCount>, cl, rtv, dsv, adjacent); }

		// When current bind slot is inside BufferPack then only one call for first resource is required in case of resource adjacency.
		// Resources are considered adjacent when during creation in FrameBufferDesc they have been specified one by one.
		// In case of another resource between them that is not used as SRV or UAV then such resources are still adjacent.
		// UAV resources are only adjacent in situation of following resources, ex:
		// 1: SRV, 2: UAV/SRV, 3: SRV
		// Resources 2 and 3 are adjacent and can be set in one call, but resources 1 and 2 are not and require separate BufferPacks.
		// WARNING! Resources with higher mips levels are never adjacent with resources created after them!
		constexpr void SetSRV(CommandList& cl, Binding::Context& bindCtx, RID rid) const noexcept { ZE_API_BACKEND_CALL(SetSRV, cl, bindCtx, rid); }
		// When current bind slot is inside BufferPack then only one call for first resource is required in case of resource adjacency.
		// Resources are considered adjacent when during creation in FrameBufferDesc they have been specified one by one.
		// In case of another resource between them that is not used as SRV or UAV then such resources are still adjacent.
		// UAV resources are only adjacent in situation of following resources, ex:
		// 1: SRV, 2: UAV/SRV, 3: SRV
		// Resources 2 and 3 are adjacent and can be set in one call, but resources 1 and 2 are not and require separate BufferPacks.
		// WARNING! Resources with higher mips levels are never adjacent with resources created after them!
		constexpr void SetUAV(CommandList& cl, Binding::Context& bindCtx, RID rid) const noexcept { ZE_API_BACKEND_CALL(SetUAV, cl, bindCtx, rid); }
		constexpr void SetUAV(CommandList& cl, Binding::Context& bindCtx, RID rid, U16 mipLevel) const noexcept { ZE_API_BACKEND_CALL(SetUAV, cl, bindCtx, rid, mipLevel); }

		// Perform barrier between 2 usages of resource as UAV
		constexpr void BarrierUAV(CommandList& cl, RID rid) const noexcept { ZE_API_BACKEND_CALL(BarrierUAV, cl, rid); }
		// Manually transitions resource between states, recomended to use only on innner resources!
		constexpr void BarrierTransition(CommandList& cl, RID rid, Resource::State before, Resource::State after) const noexcept { ZE_API_BACKEND_CALL(BarrierTransition, cl, rid, before, after); }
		// Manually transitions resource between states, recomended to use only on innner resources!
		template<U32 BarrierCount>
		constexpr void BarrierTransition(CommandList& cl, const std::array<TransitionInfo, BarrierCount>& barriers) const noexcept { ZE_API_BACKEND_CALL(BarrierTransition<BarrierCount>, cl, barriers); }

		// Render target before first use must be initialized, cleared or overwritten by full resource copy (except backbuffer)
		constexpr void ClearRTV(CommandList& cl, RID rid, const ColorF4& color) const noexcept { ZE_API_BACKEND_CALL(ClearRTV, cl, rid, color); }
		// Depth stencil before first use must be initialized, cleared or overwritten by full resource copy
		constexpr void ClearDSV(CommandList& cl, RID rid, float depth, U8 stencil) const noexcept { ZE_API_BACKEND_CALL(ClearDSV, cl, rid, depth, stencil); }
		constexpr void ClearUAV(CommandList& cl, RID rid, const ColorF4& color) const noexcept { ZE_API_BACKEND_CALL(ClearUAV, cl, rid, color); }
		constexpr void ClearUAV(CommandList& cl, RID rid, const Pixel colors[4]) const noexcept { ZE_API_BACKEND_CALL(ClearUAV, cl, rid, colors); }

		constexpr void SwapBackbuffer(Device& dev, SwapChain& swapChain) noexcept { ZE_API_BACKEND_CALL(SwapBackbuffer, dev, swapChain); }
		constexpr void InitTransitions(Device& dev, CommandList& cl) const { ZE_API_BACKEND_CALL(InitTransitions, dev, cl); }
		constexpr void ExitTransitions(Device& dev, CommandList& cl, U64 level) const { ZE_API_BACKEND_CALL(ExitTransitions, dev, cl, level); }
	};
}