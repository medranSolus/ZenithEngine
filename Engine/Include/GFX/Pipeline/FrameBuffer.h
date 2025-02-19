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

		constexpr void Init(Device& dev, const FrameBufferDesc& desc) { ZE_RHI_BACKEND_VAR.Init(dev, desc); }
		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, const FrameBufferDesc& desc) { ZE_RHI_BACKEND_VAR.Switch(nextApi, dev, desc); }
		ZE_RHI_BACKEND_GET(Pipeline::FrameBuffer);

		// Main Gfx API

		// Get width and height of the resource
		constexpr UInt2 GetDimmensions(RID rid) const noexcept { UInt2 dimm; ZE_RHI_BACKEND_CALL_RET(dimm, GetDimmensions, rid); return dimm; }
		constexpr U16 GetArraySize(RID rid) const noexcept { U16 arraySize; ZE_RHI_BACKEND_CALL_RET(arraySize, GetArraySize, rid); return arraySize; }
		constexpr U16 GetMipCount(RID rid) const noexcept { U16 mips; ZE_RHI_BACKEND_CALL_RET(mips, GetMipCount, rid); return mips; }
		constexpr PixelFormat GetFormat(RID rid) const noexcept { PixelFormat format; ZE_RHI_BACKEND_CALL_RET(format, GetFormat, rid); return format; }
		constexpr bool IsUAV(RID rid) const noexcept { bool val; ZE_RHI_BACKEND_CALL_RET(val, IsUAV, rid); return val; }
		constexpr bool IsCubeTexture(RID rid) const noexcept { bool val; ZE_RHI_BACKEND_CALL_RET(val, IsCubeTexture, rid); return val; }
		constexpr bool IsTexture1D(RID rid) const noexcept { bool val; ZE_RHI_BACKEND_CALL_RET(val, IsTexture1D, rid); return val; }
		constexpr bool IsTexture3D(RID rid) const noexcept { bool val; ZE_RHI_BACKEND_CALL_RET(val, IsTexture3D, rid); return val; }
		constexpr bool IsBuffer(RID rid) const noexcept { bool val; ZE_RHI_BACKEND_CALL_RET(val, IsBuffer, rid); return val; }
		constexpr bool IsArrayView(RID rid) const noexcept { bool val; ZE_RHI_BACKEND_CALL_RET(val, IsArrayView, rid); return val; }

		// When render targets have been created one after one without any depth stencil between them
		// they are considered adjacent which can speed-up their setting in the pipeline.
		// WARNING! Resources with higher mips levels are never adjacent with resources created after them!
		template<U8 RTVCount>
		constexpr void BeginRaster(CommandList& cl, const RID* rtv, bool adjacent = false) const noexcept { ZE_RHI_BACKEND_CALL(BeginRaster<RTVCount>, cl, rtv, adjacent); }
		// When render targets have been created one after one without any depth stencil between them
		// they are considered adjacent which can speed-up their setting in the pipeline.
		// WARNING! Resources with higher mips levels are never adjacent with resources created after them!
		template<U8 RTVCount>
		constexpr void BeginRaster(CommandList& cl, const RID* rtv, RID dsv, bool adjacent = false) const noexcept { ZE_RHI_BACKEND_CALL(BeginRaster<RTVCount>, cl, rtv, dsv, adjacent); }

		// Start rasterization when render target list may contain not present resources
		constexpr void BeginRasterSparse(CommandList& cl, const RID* rtv, U8 count) const noexcept { ZE_RHI_BACKEND_CALL(BeginRasterSparse, cl, rtv, count); }
		// Start rasterization when render target list may contain not present resources
		constexpr void BeginRasterSparse(CommandList& cl, const RID* rtv, RID dsv, U8 count) const noexcept { ZE_RHI_BACKEND_CALL(BeginRasterSparse, cl, rtv, dsv, count); }

		// Maybe add also ability to set scale and offset for viewport if needed
		constexpr void BeginRasterDepthOnly(CommandList& cl, RID dsv) const noexcept { ZE_RHI_BACKEND_CALL(BeginRasterDepthOnly, cl, dsv); }
		constexpr void BeginRaster(CommandList& cl, RID rtv, RID dsv = INVALID_RID) const noexcept { ZE_RHI_BACKEND_CALL(BeginRaster, cl, rtv, dsv); }

		constexpr void BeginRasterDepthOnly(CommandList& cl, RID dsv, U16 mipLevel) const noexcept { ZE_RHI_BACKEND_CALL(BeginRasterDepthOnly, cl, dsv, mipLevel); }
		constexpr void BeginRaster(CommandList& cl, RID rtv, RID dsv, U16 mipLevel) const noexcept { ZE_RHI_BACKEND_CALL(BeginRaster, cl, rtv, dsv, mipLevel); }

		// When current bind slot is inside BufferPack then only one call for first resource is required in case of resource adjacency.
		// Resources are considered adjacent when during creation in render graph they have been specified one by one.
		// Resource adjacency is based on type of view type, SRV and UAV are grouped separately, ex:
		// 1: SRV/UAV, 2: SRV, 3: SRV/UAV
		// Resources 1, 2 and 3 are adjacent in SRV group and can be set in one call, while resources 1 and 3 are still adjacent in UAV group.
		// WARNING! Resources with higher mips levels are never adjacent with resources created after them!
		constexpr void SetSRV(CommandList& cl, Binding::Context& bindCtx, RID rid) const noexcept { ZE_RHI_BACKEND_CALL(SetSRV, cl, bindCtx, rid); }
		// When current bind slot is inside BufferPack then only one call for first resource is required in case of resource adjacency.
		// Resources are considered adjacent when during creation in render graph they have been specified one by one.
		// Resource adjacency is based on type of view type, SRV and UAV are grouped separately, ex:
		// 1: SRV/UAV, 2: SRV, 3: SRV/UAV
		// Resources 1, 2 and 3 are adjacent in SRV group and can be set in one call, while resources 1 and 3 are still adjacent in UAV group.
		// WARNING! Resources with higher mips levels are never adjacent with resources created after them!
		constexpr void SetUAV(CommandList& cl, Binding::Context& bindCtx, RID rid) const noexcept { ZE_RHI_BACKEND_CALL(SetUAV, cl, bindCtx, rid); }
		constexpr void SetUAV(CommandList& cl, Binding::Context& bindCtx, RID rid, U16 mipLevel) const noexcept { ZE_RHI_BACKEND_CALL(SetUAV, cl, bindCtx, rid, mipLevel); }
		constexpr void SetResourceNGX(NVSDK_NGX_Parameter* param, std::string_view name, RID res) const noexcept { ZE_RHI_BACKEND_CALL(SetResourceNGX, param, name, res); }

		// All begin rasterization commands must end with this function so proper handling of render passes is ensured
		constexpr void EndRaster(CommandList& cl) const noexcept { ZE_RHI_BACKEND_CALL(EndRaster, cl); }

		constexpr void ClearRTV(CommandList& cl, RID rid, const ColorF4& color) const noexcept { ZE_RHI_BACKEND_CALL(ClearRTV, cl, rid, color); }
		constexpr void ClearDSV(CommandList& cl, RID rid, float depth, U8 stencil) const noexcept { ZE_RHI_BACKEND_CALL(ClearDSV, cl, rid, depth, stencil); }
		constexpr void ClearUAV(CommandList& cl, RID rid, const ColorF4& color) const noexcept { ZE_RHI_BACKEND_CALL(ClearUAV, cl, rid, color); }
		constexpr void ClearUAV(CommandList& cl, RID rid, const Pixel colors[4]) const noexcept { ZE_RHI_BACKEND_CALL(ClearUAV, cl, rid, colors); }

		constexpr void Copy(Device& dev, CommandList& cl, RID src, RID dest) const noexcept { ZE_RHI_BACKEND_CALL(Copy, dev, cl, src, dest); }
		constexpr void CopyFullResource(CommandList& cl, RID src, RID dest) const noexcept { ZE_RHI_BACKEND_CALL(CopyFullResource, cl, src, dest); }
		constexpr void CopyBufferRegion(CommandList& cl, RID src, U64 srcOffset, RID dest, U64 destOffset, U64 bytes) const noexcept { ZE_RHI_BACKEND_CALL(CopyBufferRegion, cl, src, srcOffset, dest, destOffset, bytes); }

		constexpr void InitResource(CommandList& cl, RID rid, const Resource::CBuffer& buffer) const noexcept { ZE_RHI_BACKEND_CALL(InitResource, cl, rid, buffer); }
		constexpr void InitResource(CommandList& cl, RID rid, const Resource::Texture::Pack& texture, U32 index) const noexcept { ZE_RHI_BACKEND_CALL(InitResource, cl, rid, texture, index); }

		// Manually transition resources between layouts and accesses in pipeline, recomended to use only on innner resources!
		template<U32 BarrierCount>
		constexpr void Barrier(CommandList& cl, const std::array<BarrierTransition, BarrierCount>& barriers) const noexcept { ZE_RHI_BACKEND_CALL(Barrier<BarrierCount>, cl, barriers); }
		// Manually transition resource between layout and access in pipeline, recomended to use only on innner resources!
		constexpr void Barrier(CommandList& cl, const BarrierTransition* barriers, U32 count) const noexcept { ZE_RHI_BACKEND_CALL(Barrier, cl, barriers, count); }
		// Manually transition resource between layout and access in pipeline, recomended to use only on innner resources!
		constexpr void Barrier(CommandList& cl, const BarrierTransition& desc) const noexcept { ZE_RHI_BACKEND_CALL(Barrier, cl, desc); }

		constexpr void MapResource(Device& dev, RID rid, void** ptr) const { ZE_RHI_BACKEND_CALL(MapResource, dev, rid, ptr); }
		constexpr void UnmapResource(RID rid) const noexcept { ZE_RHI_BACKEND_CALL(UnmapResource, rid); }

		constexpr FfxApiResource GetFfxResource(RID rid, U32 state) const noexcept { FfxApiResource res = {}; ZE_RHI_BACKEND_CALL_RET(res, GetFfxResource, rid, state); return res; }

		// Depth, exposure and responsive parameters are optional, when this buffers are not present then pass in INALID_RID
		constexpr void ExecuteXeSS(Device& dev, CommandList& cl, RID color, RID motionVectors, RID depth, RID exposure, RID responsive, RID output, float jitterX, float jitterY, bool reset) const { ZE_RHI_BACKEND_CALL(ExecuteXeSS, dev, cl, color, motionVectors, depth, exposure, responsive, output, jitterX, jitterY, reset); }
		constexpr void ExecuteIndirect(CommandList& cl, CommandSignature& signature, RID commandsBuffer, U32 commandsOffset) const noexcept { ZE_RHI_BACKEND_CALL(ExecuteIndirect, cl, signature, commandsBuffer, commandsOffset); }

		constexpr void SwapBackbuffer(Device& dev, SwapChain& swapChain) noexcept { ZE_RHI_BACKEND_CALL(SwapBackbuffer, dev, swapChain); }
		// Before destroying FrameBuffer you have to call this function for proper memory freeing
		constexpr void Free(Device& dev) noexcept { ZE_RHI_BACKEND_CALL(Free, dev); }
	};
}