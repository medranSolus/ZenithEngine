#pragma once
#include "GFX/Binding/Context.h"
#include "GFX/Resource/Texture/Pack.h"
#include "GFX/Resource/CBuffer.h"
#include "GFX/Pipeline/FrameBufferDesc.h"
#include "GFX/CommandSignature.h"
#include "GFX/SwapChain.h"
ZE_WARNING_PUSH
#include "nvsdk_ngx_params.h"
#include "ffx_api/ffx_api_types.h"
ZE_WARNING_POP

namespace ZE::RHI::DX12::Pipeline
{
	class FrameBuffer final
	{
		struct ResourceInitInfo
		{
			RID Handle;
			U32 Chunks;
			U32 ChunkOffset;
			D3D12_RESOURCE_DESC1 Desc;
			D3D12_CLEAR_VALUE ClearVal;
			U32 ByteStride;
			std::bitset<8> Flags;

			constexpr bool IsCube() const noexcept { return Flags[0]; }
			constexpr void SetCube() noexcept { Flags[0] = true; }
			constexpr bool UseStencilView() const noexcept { return Flags[1]; }
			constexpr void SetStencilView() noexcept { Flags[1] = true; }
			constexpr bool IsRawBufferView() const noexcept { return Flags[2]; }
			constexpr void SetRawBufferView() noexcept { Flags[2] = true; }
			constexpr bool IsTemporal() const noexcept { return Flags[3]; }
			constexpr void SetTemporal() noexcept { Flags[3] = true; }
			constexpr bool IsHeapUAV() const noexcept { return Flags[4]; }
			constexpr void SetHeapUAV() noexcept { Flags[4] = true; }
			constexpr bool IsHeapBuffer() const noexcept { return Flags[5]; }
			constexpr void SetHeapBuffer() noexcept { Flags[5] = true; }
			constexpr bool IsArrayView() const noexcept { return Flags[6]; }
			constexpr void ForceArrayView() noexcept { Flags[6] = true; }
			constexpr bool IsMemoryOnlyRegion() const noexcept { return Flags[7]; }
			constexpr void SetMemoryOnlyRegion() noexcept { Flags[7] = true; }
		};
		struct BufferData
		{
			DX::ComPtr<IResource> Resource;
			UInt2 Size;
			U16 Array;
			U16 Mips;
			PixelFormat Format;
			D3D12_RESOURCE_DIMENSION Dimenions;
			std::bitset<3> Flags;

			constexpr bool IsCube() const noexcept { return Flags[0]; }
			constexpr void SetCube() noexcept { Flags[0] = true; }
			constexpr bool IsArrayView() const noexcept { return Flags[1]; }
			constexpr void SetArrayView() noexcept { Flags[1] = true; }
			// If true then Size contains size in bytes with X being LSB and Y being MSB parts, Array contain LSB of chunk offset and Mips contains MSB part
			constexpr bool IsMemoryOnlyRegion() const noexcept { return Flags[2]; }
			constexpr void SetMemoryOnlyRegion() noexcept { Flags[2] = true; }
		};
		struct HandleSRV
		{
			D3D12_CPU_DESCRIPTOR_HANDLE CpuShaderVisibleHandle;
			D3D12_GPU_DESCRIPTOR_HANDLE GpuShaderVisibleHandle;
		};
		struct HandleUAV
		{
			D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle;
			D3D12_CPU_DESCRIPTOR_HANDLE CpuShaderVisibleHandle;
			D3D12_GPU_DESCRIPTOR_HANDLE GpuShaderVisibleHandle;
		};

#if !_ZE_MODE_RELEASE
		mutable bool isRasterActive = false;
#endif
		RID resourceCount;
		Ptr<BufferData> resources;
		Ptr<D3D12_CPU_DESCRIPTOR_HANDLE> rtvDsvHandles;
		Ptr<Ptr<D3D12_CPU_DESCRIPTOR_HANDLE>> rtvDsvMips; // No backbuffer
		Ptr<HandleSRV> srvHandles;
		Ptr<HandleUAV> uavHandles; // No backbuffer
		Ptr<Ptr<HandleUAV>> uavMips; // No backbuffer

		DX::ComPtr<IDescriptorHeap> rtvDescHeap;
		DX::ComPtr<IDescriptorHeap> dsvDescHeap;
		DX::ComPtr<IHeap> mainHeap;
		DX::ComPtr<IHeap> uavHeap;
		DX::ComPtr<IHeap> bufferHeap;
		DescriptorInfo descInfo;
		DescriptorInfo descInfoCpu;

#if !_ZE_MODE_RELEASE
		static void PrintMemory(std::string&& memID, U32 levelCount, U64 heapSize,
			std::vector<ResourceInitInfo>::iterator resBegin, std::vector<ResourceInitInfo>::iterator resEnd,
			const std::vector<std::pair<U32, U32>>& resourcesLifetime) noexcept;
#endif
		static U64 AllocateResources(std::vector<ResourceInitInfo>::iterator resBegin, std::vector<ResourceInitInfo>::iterator resEnd,
			const std::vector<std::pair<U32, U32>>& resourcesLifetime, U32 levelCount, GFX::Pipeline::FrameBufferFlags flags) noexcept;

		void EnterRaster() const noexcept;
		void SetupViewport(D3D12_VIEWPORT& viewport, D3D12_RECT& scissorRect, RID rid) const noexcept;
		void SetViewport(CommandList& cl, RID rid) const noexcept;
		void FillBarier(D3D12_TEXTURE_BARRIER& barrier, const GFX::Pipeline::BarrierTransition& desc) const noexcept;
		void PerformBarrier(CommandList& cl, const D3D12_TEXTURE_BARRIER* barriers, U32 count) const noexcept;

	public:
		FrameBuffer() = default;
		FrameBuffer(GFX::Device& dev, const GFX::Pipeline::FrameBufferDesc& desc);
		ZE_CLASS_DELETE(FrameBuffer);
		~FrameBuffer();

		constexpr UInt2 GetDimmensions(RID rid) const noexcept { ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!"); return resources[rid].Size; }
		constexpr U16 GetArraySize(RID rid) const noexcept { ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!"); return resources[rid].Array; }
		constexpr U16 GetMipCount(RID rid) const noexcept { ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!"); return resources[rid].Mips; }
		constexpr PixelFormat GetFormat(RID rid) const noexcept { ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!"); return resources[rid].Format; }
		constexpr bool IsUAV(RID rid) const noexcept { ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!"); if (rid == BACKBUFFER_RID) return false; return uavHandles[rid - 1].CpuHandle.ptr != UINT64_MAX; }
		constexpr bool IsCubeTexture(RID rid) const noexcept { ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!"); return resources[rid].IsCube(); }
		constexpr bool IsTexture1D(RID rid) const noexcept { ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!"); return resources[rid].Dimenions == D3D12_RESOURCE_DIMENSION_TEXTURE1D; }
		constexpr bool IsTexture3D(RID rid) const noexcept { ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!"); return resources[rid].Dimenions == D3D12_RESOURCE_DIMENSION_TEXTURE3D; }
		constexpr bool IsBuffer(RID rid) const noexcept { ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!"); return resources[rid].Dimenions == D3D12_RESOURCE_DIMENSION_BUFFER; }
		constexpr bool IsArrayView(RID rid) const noexcept { ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!"); return resources[rid].IsArrayView(); }

		template<U8 RTVCount>
		void BeginRaster(GFX::CommandList& cl, const RID* rtv, bool adjacent) const noexcept;
		template<U8 RTVCount>
		void BeginRaster(GFX::CommandList& cl, const RID* rtv, RID dsv, bool adjacent) const noexcept;

		void BeginRasterSparse(GFX::CommandList& cl, const RID* rtv, U8 count) const noexcept;
		void BeginRasterSparse(GFX::CommandList& cl, const RID* rtv, RID dsv, U8 count) const noexcept;

		void BeginRasterDepthOnly(GFX::CommandList& cl, RID dsv) const noexcept;
		void BeginRaster(GFX::CommandList& cl, RID rtv, RID dsv) const noexcept;

		void BeginRasterDepthOnly(GFX::CommandList& cl, RID dsv, U16 mipLevel) const noexcept;
		void BeginRaster(GFX::CommandList& cl, RID rtv, RID dsv, U16 mipLevel) const noexcept;

		void SetSRV(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, RID srv) const noexcept;
		void SetUAV(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, RID uav) const noexcept;
		void SetUAV(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, RID uav, U16 mipLevel) const noexcept;
		void SetResourceNGX(NVSDK_NGX_Parameter* param, std::string_view name, RID res) const noexcept;

		void EndRaster(GFX::CommandList& cl) const noexcept;

		void ClearRTV(GFX::CommandList& cl, RID rtv, const ColorF4& color) const noexcept;
		void ClearDSV(GFX::CommandList& cl, RID dsv, float depth, U8 stencil) const noexcept;
		void ClearUAV(GFX::CommandList& cl, RID uav, const ColorF4& color) const noexcept;
		void ClearUAV(GFX::CommandList& cl, RID uav, const Pixel colors[4]) const noexcept;

		void Copy(GFX::Device& dev, GFX::CommandList& cl, RID src, RID dest) const noexcept;
		void CopyFullResource(GFX::CommandList& cl, RID src, RID dest) const noexcept;
		void CopyBufferRegion(GFX::CommandList& cl, RID src, U64 srcOffset, RID dest, U64 destOffset, U64 bytes) const noexcept;

		void InitResource(GFX::CommandList& cl, RID rid, const GFX::Resource::CBuffer& buffer) const noexcept;
		void InitResource(GFX::CommandList& cl, RID rid, const GFX::Resource::Texture::Pack& texture, U32 index) const noexcept;

		template<U32 BarrierCount>
		void Barrier(GFX::CommandList& cl, const std::array<GFX::Pipeline::BarrierTransition, BarrierCount>& barriers) const noexcept;
		void Barrier(GFX::CommandList& cl, const GFX::Pipeline::BarrierTransition* barriers, U32 count) const noexcept;
		void Barrier(GFX::CommandList& cl, const GFX::Pipeline::BarrierTransition& desc) const noexcept;

		void MapResource(GFX::Device& dev, RID rid, void** ptr) const;
		void UnmapResource(RID rid) const noexcept;

		FfxApiResource GetFfxResource(RID rid, U32 state) const noexcept;

		void ExecuteXeSS(GFX::Device& dev, GFX::CommandList& cl, RID color, RID motionVectors, RID depth,
			RID exposure, RID responsive, RID output, float jitterX, float jitterY, bool reset) const;
		void ExecuteIndirect(GFX::CommandList& cl, GFX::CommandSignature& signature, RID commandsBuffer, U32 commandsOffset) const noexcept;
		void SwapBackbuffer(GFX::Device& dev, GFX::SwapChain& swapChain) noexcept;
		void Free(GFX::Device& dev) noexcept;

		// Gfx API Internal

		DX::ComPtr<IResource> GetResource(RID rid) const noexcept { ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!"); return resources[rid].Resource; }
		const HandleSRV& GetSRV(RID srv) const noexcept { ZE_ASSERT(srv < resourceCount, "Resource ID outside available range!"); return srvHandles[srv]; }
		const HandleUAV& GetUAV(RID uav) const noexcept { ZE_ASSERT(uav < resourceCount, "Resource ID outside available range!"); ZE_ASSERT(uav != BACKBUFFER_RID, "Cannot use backbuffer as unnordered access!"); return uavHandles[uav - 1]; }
	};

#pragma region Functions
	template<U8 RTVCount>
	void FrameBuffer::BeginRaster(GFX::CommandList& cl, const RID* rtv, bool adjacent) const noexcept
	{
		static_assert(RTVCount > 1, "For performance reasons FrameBuffer::BeginRaster() should be only used for multiple render targets!");
		static_assert(RTVCount <= Settings::MAX_RENDER_TARGETS, "Exceeding max number of concurrently bound render targets!");
		EnterRaster();

		D3D12_CPU_DESCRIPTOR_HANDLE handles[RTVCount];
		D3D12_VIEWPORT vieports[RTVCount];
		D3D12_RECT scissorRects[RTVCount];
		for (U32 i = 0; i < RTVCount; ++i)
		{
			RID id = rtv[i];
			ZE_ASSERT(id < resourceCount, "Resource ID outside available range!");

			handles[i] = rtvDsvHandles[id];
			ZE_ASSERT(handles[i].ptr != UINT64_MAX, "Current resource is not suitable for being render target!");
			SetupViewport(vieports[i], scissorRects[i], id);
		}
		cl.Get().dx12.GetList()->RSSetViewports(RTVCount, vieports);
		cl.Get().dx12.GetList()->RSSetScissorRects(RTVCount, scissorRects);
		cl.Get().dx12.GetList()->OMSetRenderTargets(RTVCount, handles, adjacent, nullptr);
	}

	template<U8 RTVCount>
	void FrameBuffer::BeginRaster(GFX::CommandList& cl, const RID* rtv, RID dsv, bool adjacent) const noexcept
	{
		static_assert(RTVCount > 1, "For performance reasons FrameBuffer::BeginRaster() should be only used for multiple render targets!");
		static_assert(RTVCount <= Settings::MAX_RENDER_TARGETS, "Exceeding max number of concurrently bound render targets!");
		ZE_ASSERT(dsv < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(dsv != BACKBUFFER_RID, "Cannot use backbuffer as depth stencil!");
		ZE_ASSERT(rtvDsvHandles[dsv].ptr != UINT64_MAX, "Current resource is not suitable for being depth stencil!");
		EnterRaster();

		D3D12_CPU_DESCRIPTOR_HANDLE handles[RTVCount];
		D3D12_VIEWPORT vieports[RTVCount];
		D3D12_RECT scissorRects[RTVCount];
		for (U32 i = 0; i < RTVCount; ++i)
		{
			RID id = rtv[i];
			ZE_ASSERT(id < resourceCount, "Resource ID outside available range!");

			handles[i] = rtvDsvHandles[id];
			ZE_ASSERT(handles[i].ptr != UINT64_MAX, "Current resource is not suitable for being render target!");
			SetupViewport(vieports[i], scissorRects[i], id);
		}
		cl.Get().dx12.GetList()->RSSetViewports(RTVCount, vieports);
		cl.Get().dx12.GetList()->RSSetScissorRects(RTVCount, scissorRects);
		cl.Get().dx12.GetList()->OMSetRenderTargets(RTVCount, handles, adjacent, rtvDsvHandles + dsv);
	}

	template<U32 BarrierCount>
	void FrameBuffer::Barrier(GFX::CommandList& cl, const std::array<GFX::Pipeline::BarrierTransition, BarrierCount>& barriers) const noexcept
	{
		static_assert(BarrierCount > 1, "For performance reasons FrameBuffer::Barrier() should be only used for multiple barriers!");

		D3D12_TEXTURE_BARRIER texBarriers[BarrierCount];
		for (U32 i = 0; i < BarrierCount; ++i)
			FillBarier(texBarriers[i], barriers.at(i));
		PerformBarrier(cl.Get().dx12, texBarriers, BarrierCount);
	}
#pragma endregion
}