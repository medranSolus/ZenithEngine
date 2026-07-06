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
		ZE_CLASS_MOVE(FrameBuffer);
		~FrameBuffer() = default;

		static Expected<FrameBuffer> Create(Device& dev, const FrameBufferDesc& desc) noexcept { ZE_RHI_BACKEND_CREATE(Pipeline::FrameBuffer, dev, desc); }
		ZE_RHI_BACKEND_GET(Pipeline::FrameBuffer);

		// Main Gfx API

		// Get width and height of the resource
		constexpr UInt2 GetDimmensions(RID rid) const noexcept { ZE_RHI_BACKEND_CALL_RET(GetDimmensions, rid); }
		constexpr U16 GetArraySize(RID rid) const noexcept { ZE_RHI_BACKEND_CALL_RET(GetArraySize, rid); }
		constexpr U16 GetMipCount(RID rid) const noexcept { ZE_RHI_BACKEND_CALL_RET(GetMipCount, rid); }
		constexpr PixelFormat GetFormat(RID rid) const noexcept { ZE_RHI_BACKEND_CALL_RET(GetFormat, rid); }
		constexpr bool IsUAV(RID rid) const noexcept { ZE_RHI_BACKEND_CALL_RET(IsUAV, rid); }
		constexpr bool IsCubeTexture(RID rid) const noexcept { ZE_RHI_BACKEND_CALL_RET(IsCubeTexture, rid); }
		constexpr bool IsTexture1D(RID rid) const noexcept { ZE_RHI_BACKEND_CALL_RET(IsTexture1D, rid); }
		constexpr bool IsTexture3D(RID rid) const noexcept { ZE_RHI_BACKEND_CALL_RET(IsTexture3D, rid); }
		constexpr bool IsBuffer(RID rid) const noexcept { ZE_RHI_BACKEND_CALL_RET(IsBuffer, rid); }
		constexpr bool IsArrayView(RID rid) const noexcept { ZE_RHI_BACKEND_CALL_RET(IsArrayView, rid); }

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

		Status RegisterOutsideResource(RID rid, Resource::Texture::Pack& textures, U32 textureIndex, FrameResourceType type) noexcept { ZE_RHI_BACKEND_CALL_RET(RegisterOutsideResource, rid, textures, textureIndex, type); }

		Status MapResource(Device& dev, RID rid, void** ptr) const noexcept { ZE_RHI_BACKEND_CALL_RET(MapResource, dev, rid, ptr); }
		constexpr void UnmapResource(RID rid) const noexcept { ZE_RHI_BACKEND_CALL(UnmapResource, rid); }

#if _ZE_FFX_API_ENABLED
		constexpr FfxApiResource GetFfxResource(RID rid, U32 state) const noexcept { ZE_RHI_BACKEND_CALL_RET(GetFfxResource, rid, state); }
#endif

		Status ExecuteIndirect(CommandList& cl, CommandSignature& signature, RID commandsBuffer, U32 commandsOffset) const noexcept { ZE_RHI_BACKEND_CALL_RET(ExecuteIndirect, cl, signature, commandsBuffer, commandsOffset); }
		Status SwapBackbuffer(Device& dev, SwapChain& swapChain) noexcept { ZE_RHI_BACKEND_CALL_RET(SwapBackbuffer, dev, swapChain); }
	};
}