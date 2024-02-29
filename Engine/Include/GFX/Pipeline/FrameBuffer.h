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

		constexpr void Init(Device& dev, CommandList& mainList, FrameBufferDesc& desc) { ZE_RHI_BACKEND_VAR.Init(dev, mainList, desc); }
		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, CommandList& mainList) { /*ZE_RHI_BACKEND_VAR.Switch(nextApi, dev, mainList);*/ }
		ZE_RHI_BACKEND_GET(Pipeline::FrameBuffer);

		// Main Gfx API

		// Get width and height of the resource
		constexpr UInt2 GetDimmensions(RID rid) const noexcept { UInt2 dimm; ZE_RHI_BACKEND_CALL_RET(dimm, GetDimmensions, rid); return dimm; }
		constexpr U16 GetArraySize(RID rid) const noexcept { U16 arraySize; ZE_RHI_BACKEND_CALL_RET(arraySize, GetArraySize, rid); return arraySize; }
		constexpr U16 GetMipCount(RID rid) const noexcept { U16 mips; ZE_RHI_BACKEND_CALL_RET(mips, GetMipCount, rid); return mips; }
		constexpr bool IsCubeTexture(RID rid) const noexcept { bool val; ZE_RHI_BACKEND_CALL_RET(val, IsCubeTexture, rid); return val; }
		constexpr bool IsArrayView(RID rid) const noexcept { bool val; ZE_RHI_BACKEND_CALL_RET(val, IsArrayView, rid); return val; }
		constexpr bool IsUAV(RID rid) const noexcept { bool val; ZE_RHI_BACKEND_CALL_RET(val, IsUAV, rid); return val; }
		constexpr PixelFormat GetFormat(RID rid) const noexcept { PixelFormat format; ZE_RHI_BACKEND_CALL_RET(format, GetFormat, rid); return format; }

		// Render target before first use must be initialized, cleared or overwritten by full resource copy (except backbuffer)
		constexpr void InitRTV(CommandList& cl, RID rid) const noexcept { ZE_RHI_BACKEND_CALL(InitRTV, cl, rid); }
		// Depth stencil before first use must be initialized, cleared or overwritten by full resource copy
		constexpr void InitDSV(CommandList& cl, RID rid) const noexcept { ZE_RHI_BACKEND_CALL(InitDSV, cl, rid); }

		// Render targets (except backbuffer) and Depth stencils before first use must be initialized, cleared or overwritten by full resource copy
		constexpr void Copy(CommandList& cl, RID src, RID dest) const noexcept { ZE_RHI_BACKEND_CALL(Copy, cl, src, dest); }

		// Maybe add also ability to set scale and offset for viewport if needed
		constexpr void SetRTV(CommandList& cl, RID rid) const noexcept { ZE_RHI_BACKEND_CALL(SetRTV, cl, rid); }
		constexpr void SetRTV(CommandList& cl, RID rid, U16 mipLevel) const noexcept { ZE_RHI_BACKEND_CALL(SetRTV, cl, rid, mipLevel); }
		constexpr void SetDSV(CommandList& cl, RID rid) const noexcept { ZE_RHI_BACKEND_CALL(SetDSV, cl, rid); }
		constexpr void SetDSV(CommandList& cl, RID rid, U16 mipLevel) const noexcept { ZE_RHI_BACKEND_CALL(SetDSV, cl, rid, mipLevel); }
		constexpr void SetOutput(CommandList& cl, RID rtv, RID dsv) const noexcept { ZE_RHI_BACKEND_CALL(SetOutput, cl, rtv, dsv); }

		// Bind RTV when resource list is not adjanced in memory or may contain not present resources
		void SetRTVSparse(GFX::CommandList& cl, const RID* rid, U8 count) const noexcept { ZE_RHI_BACKEND_CALL(SetRTVSparse, cl, rid, count); }
		// Specify output when resource list is not adjanced in memory or may contain not present resources
		void SetOutputSparse(GFX::CommandList& cl, const RID* rtv, RID dsv, U8 count) const noexcept { ZE_RHI_BACKEND_CALL(SetOutputSparse, cl, rtv, dsv, count); }

		// When render targets have been created one after one without any depth stencil between them
		// they are considered adjacent which can speed-up their setting in the pipeline.
		// WARNING! Resources with higher mips levels are never adjacent with resources created after them!
		template<U32 RTVCount>
		constexpr void SetRTV(CommandList& cl, const RID* rid, bool adjacent = false) const noexcept { ZE_RHI_BACKEND_CALL(SetRTV<RTVCount>, cl, rid, adjacent); }
		// When render targets have been created one after one without any depth stencil between them
		// they are considered adjacent which can speed-up their setting in the pipeline.
		// WARNING! Resources with higher mips levels are never adjacent with resources created after them!
		template<U32 RTVCount>
		constexpr void SetOutput(CommandList& cl, const RID* rtv, RID dsv, bool adjacent = false) const noexcept { ZE_RHI_BACKEND_CALL(SetOutput<RTVCount>, cl, rtv, dsv, adjacent); }

		// When current bind slot is inside BufferPack then only one call for first resource is required in case of resource adjacency.
		// Resources are considered adjacent when during creation in FrameBufferDesc they have been specified one by one.
		// Resource adjacency is based on type of view type, SRV and UAV are grouped separately, ex:
		// 1: SRV/UAV, 2: SRV, 3: SRV/UAV
		// Resources 1, 2 and 3 are adjacent in SRV group and can be set in one call, while resources 1 and 3 are still adjacent in UAV group.
		// WARNING! Resources with higher mips levels are never adjacent with resources created after them!
		constexpr void SetSRV(CommandList& cl, Binding::Context& bindCtx, RID rid) const noexcept { ZE_RHI_BACKEND_CALL(SetSRV, cl, bindCtx, rid); }
		// When current bind slot is inside BufferPack then only one call for first resource is required in case of resource adjacency.
		// Resources are considered adjacent when during creation in FrameBufferDesc they have been specified one by one.
		// Resource adjacency is based on type of view type, SRV and UAV are grouped separately, ex:
		// 1: SRV/UAV, 2: SRV, 3: SRV/UAV
		// Resources 1, 2 and 3 are adjacent in SRV group and can be set in one call, while resources 1 and 3 are still adjacent in UAV group.
		// WARNING! Resources with higher mips levels are never adjacent with resources created after them!
		constexpr void SetUAV(CommandList& cl, Binding::Context& bindCtx, RID rid) const noexcept { ZE_RHI_BACKEND_CALL(SetUAV, cl, bindCtx, rid); }
		constexpr void SetUAV(CommandList& cl, Binding::Context& bindCtx, RID rid, U16 mipLevel) const noexcept { ZE_RHI_BACKEND_CALL(SetUAV, cl, bindCtx, rid, mipLevel); }

		// Perform barrier between 2 usages of resource as UAV
		constexpr void BarrierUAV(CommandList& cl, RID rid) const noexcept { ZE_RHI_BACKEND_CALL(BarrierUAV, cl, rid); }
		// Manually transitions resource between states, recomended to use only on innner resources!
		constexpr void BarrierTransition(CommandList& cl, RID rid, Resource::State before, Resource::State after) const noexcept { ZE_RHI_BACKEND_CALL(BarrierTransition, cl, rid, before, after); }
		// Manually transitions resource between states, recomended to use only on innner resources!
		template<U32 BarrierCount>
		constexpr void BarrierTransition(CommandList& cl, const std::array<TransitionInfo, BarrierCount>& barriers) const noexcept { ZE_RHI_BACKEND_CALL(BarrierTransition<BarrierCount>, cl, barriers); }

		// Render target before first use must be initialized, cleared or overwritten by full resource copy (except backbuffer)
		constexpr void ClearRTV(CommandList& cl, RID rid, const ColorF4& color) const noexcept { ZE_RHI_BACKEND_CALL(ClearRTV, cl, rid, color); }
		// Depth stencil before first use must be initialized, cleared or overwritten by full resource copy
		constexpr void ClearDSV(CommandList& cl, RID rid, float depth, U8 stencil) const noexcept { ZE_RHI_BACKEND_CALL(ClearDSV, cl, rid, depth, stencil); }
		constexpr void ClearUAV(CommandList& cl, RID rid, const ColorF4& color) const noexcept { ZE_RHI_BACKEND_CALL(ClearUAV, cl, rid, color); }
		constexpr void ClearUAV(CommandList& cl, RID rid, const Pixel colors[4]) const noexcept { ZE_RHI_BACKEND_CALL(ClearUAV, cl, rid, colors); }

		constexpr void SwapBackbuffer(Device& dev, SwapChain& swapChain) noexcept { ZE_RHI_BACKEND_CALL(SwapBackbuffer, dev, swapChain); }
		constexpr void InitTransitions(Device& dev, CommandList& cl) const { ZE_RHI_BACKEND_CALL(InitTransitions, dev, cl); }
		constexpr void ExitTransitions(Device& dev, CommandList& cl, U64 level) const { ZE_RHI_BACKEND_CALL(ExitTransitions, dev, cl, level); }

		// Before destroying FrameBuffer you have to call this function for proper memory freeing
		constexpr void Free(Device& dev) noexcept { ZE_RHI_BACKEND_CALL(Free, dev); }
	};
}