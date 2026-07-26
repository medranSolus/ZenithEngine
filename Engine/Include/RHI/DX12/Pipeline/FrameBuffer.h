#pragma once
#include "GFX/Binding/Context.h"
#include "GFX/Resource/Texture/Pack.h"
#include "GFX/Resource/CBuffer.h"
#include "GFX/Pipeline/FrameBufferDesc.h"
#include "GFX/CommandSignature.h"
#include "GFX/SwapChain.h"
ZE_WARNING_PUSH
#include "nvsdk_ngx_params.h"
#include "ffx_api_types.h"
ZE_WARNING_POP

namespace ZE::RHI::DX12::Pipeline
{
	class FrameBuffer final
	{
		struct ResourceInitInfo
		{
			RID Handle = INVALID_RID;
			U32 Chunks = 0;
			U32 ChunkOffset = 0;
			D3D12_RESOURCE_DESC1 Desc = {};
			D3D12_CLEAR_VALUE ClearVal = {};
			U32 ByteStride = 0;
			std::bitset<9> Flags = 0;

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
			constexpr bool IsOutsideResource() const noexcept { return Flags[8]; }
			constexpr void SetOutsideResource() noexcept { Flags[8] = true; }
		};
		struct BufferData
		{
			DX::ComPtr<IResource> Resource;
			UInt2 Size = {};
			U16 Array = 0;
			U16 Mips = 0;
			PixelFormat Format = PixelFormat::Unknown;
			std::bitset<7> Flags = 0;

			constexpr bool IsCube() const noexcept { return Flags[0]; }
			constexpr void SetCube() noexcept { Flags[0] = true; }
			constexpr bool IsArrayView() const noexcept { return Flags[1]; }
			constexpr void SetArrayView() noexcept { Flags[1] = true; }
			// If true then Size contains size in bytes with X being LSB and Y being MSB parts, Array contain LSB of chunk offset and Mips contains MSB part
			constexpr bool IsMemoryOnlyRegion() const noexcept { return Flags[2]; }
			constexpr void SetMemoryOnlyRegion() noexcept { Flags[2] = true; }
			constexpr bool IsOutsideResource() const noexcept { return Flags[3]; }
			constexpr void SetOutsideResource() noexcept { Flags[3] = true; }
			constexpr bool IsBuffer() const noexcept { return Flags[4]; }
			constexpr void SetBuffer() noexcept { Flags[4] = true; }
			constexpr bool IsTex1D() const noexcept { return Flags[5]; }
			constexpr void SetTex1D() noexcept { Flags[5] = true; }
			constexpr bool IsTex3D() const noexcept { return Flags[6]; }
			constexpr void SetTex3D() noexcept { Flags[6] = true; }

			bool IsResourceRegistered() const noexcept { return Resource != nullptr; }
		};
		struct HandleSRV
		{
			D3D12_CPU_DESCRIPTOR_HANDLE CpuShaderVisibleHandle = {};
			D3D12_GPU_DESCRIPTOR_HANDLE GpuShaderVisibleHandle = {};
		};
		struct HandleUAV
		{
			D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle = {};
			D3D12_CPU_DESCRIPTOR_HANDLE CpuShaderVisibleHandle = {};
			D3D12_GPU_DESCRIPTOR_HANDLE GpuShaderVisibleHandle = {};
		};

#if !_ZE_MODE_RELEASE
		mutable bool isRasterActive = false;
#endif
		RID resourceCount = 0;
		std::unique_ptr<BufferData[]> resources;
		std::unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE[]> rtvDsvHandles;
		std::unique_ptr<std::unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE[]>[]> rtvDsvMips; // No backbuffer
		std::unique_ptr<HandleSRV[]> srvHandles;
		std::unique_ptr<HandleUAV[]> uavHandles; // No backbuffer
		std::unique_ptr<std::unique_ptr<HandleUAV[]>[]> uavMips; // No backbuffer

		DX::ComPtr<IDescriptorHeap> rtvDescHeap;
		DX::ComPtr<IDescriptorHeap> dsvDescHeap;
		DX::ComPtr<IHeap> mainHeap;
		DX::ComPtr<IHeap> uavHeap;
		DX::ComPtr<IHeap> bufferHeap;
		DescriptorInfo descInfo = {};
		DescriptorInfo descInfoCpu = {};

#if !_ZE_MODE_RELEASE
		static void PrintMemory(std::string&& memID, U32 levelCount, U64 heapSize,
			std::vector<ResourceInitInfo>::iterator resBegin, std::vector<ResourceInitInfo>::iterator resEnd,
			const std::vector<std::pair<U32, U32>>& resourcesLifetime) noexcept;
#endif
		static U64 AllocateResources(std::vector<ResourceInitInfo>::iterator resBegin, std::vector<ResourceInitInfo>::iterator resEnd,
			const std::vector<std::pair<U32, U32>>& resourcesLifetime, U32 levelCount, GFX::Pipeline::FrameBufferFlags flags, U64 minimalChunkSize) noexcept;

		void EnterRaster() const noexcept;
		void SetupViewport(D3D12_VIEWPORT& viewport, D3D12_RECT& scissorRect, RID rid) const noexcept;
		void SetViewport(CommandList& cl, RID rid) const noexcept;
		void FillBarier(D3D12_BUFFER_BARRIER& barrier, const GFX::Pipeline::BarrierTransition& desc) const noexcept;
		void FillBarier(D3D12_TEXTURE_BARRIER& barrier, const GFX::Pipeline::BarrierTransition& desc) const noexcept;
		void PerformBarrier(CommandList& cl, const D3D12_TEXTURE_BARRIER* barriersTex, U32 countTex, const D3D12_BUFFER_BARRIER* barriersBuff, U32 countBuff) const noexcept;

	public:
		FrameBuffer() = default;
		ZE_CLASS_MOVE(FrameBuffer);
		~FrameBuffer();

		static Expected<FrameBuffer> Create(GFX::Device& dev, const GFX::Pipeline::FrameBufferDesc& desc) noexcept;

		constexpr UInt2 GetDimmensions(RID rid) const noexcept { ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!"); return resources[rid].Size; }
		constexpr U16 GetArraySize(RID rid) const noexcept { ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!"); return resources[rid].Array; }
		constexpr U16 GetMipCount(RID rid) const noexcept { ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!"); return resources[rid].Mips; }
		constexpr PixelFormat GetFormat(RID rid) const noexcept { ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!"); return resources[rid].Format; }
		constexpr bool IsUAV(RID rid) const noexcept { ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!"); if (rid == BACKBUFFER_RID) return false; return uavHandles[rid - 1].CpuHandle.ptr != UINT64_MAX; }
		constexpr bool IsCubeTexture(RID rid) const noexcept { ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!"); return resources[rid].IsCube(); }
		constexpr bool IsTexture1D(RID rid) const noexcept { ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!"); return resources[rid].IsTex1D(); }
		constexpr bool IsTexture3D(RID rid) const noexcept { ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!"); return resources[rid].IsTex3D(); }
		constexpr bool IsBuffer(RID rid) const noexcept { ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!"); return resources[rid].IsBuffer(); }
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

		void SetSRV(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, RID srv, U32 adjacentCount) const noexcept;
		void SetUAV(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, RID uav, U32 adjacentCount) const noexcept;
		void SetUAVMip(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, RID uav, U16 mipLevel) const noexcept;
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

		Status RegisterOutsideResource(RID rid, GFX::Resource::Texture::Pack& textures, U32 textureIndex, GFX::Pipeline::FrameResourceType type) noexcept;

		Status MapResource(GFX::Device& dev, RID rid, void** ptr) const noexcept;
		void UnmapResource(RID rid) const noexcept;

		FfxApiResource GetFfxResource(RID rid, U32 state) const noexcept;

		Status ExecuteIndirect(GFX::CommandList& cl, GFX::CommandSignature& signature, RID commandsBuffer, U32 commandsOffset) const noexcept;
		Status SwapBackbuffer(GFX::Device& dev, GFX::SwapChain& swapChain) noexcept;

		// Gfx API Internal

		IHeap* GetHeapMain() const noexcept { return mainHeap.Get(); }
		IHeap* GetHeapUAV() const noexcept { return uavHeap ? uavHeap.Get() : mainHeap.Get(); }
		IHeap* GetHeapBuffer() const noexcept { return bufferHeap ? bufferHeap.Get() : mainHeap.Get(); }

		DX::ComPtr<IResource> GetResource(RID rid) const noexcept { ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!"); ZE_ASSERT(resources[rid].IsResourceRegistered(), "Outside resource not registered!"); return resources[rid].Resource; }
		const D3D12_CPU_DESCRIPTOR_HANDLE& GetRTV(RID rtv) const noexcept { ZE_ASSERT(rtv < resourceCount, "Resource ID outside available range!"); return rtvDsvHandles[rtv]; }
		const D3D12_CPU_DESCRIPTOR_HANDLE& GetDSV(RID dsv) const noexcept { ZE_ASSERT(dsv < resourceCount, "Resource ID outside available range!"); ZE_ASSERT(dsv != BACKBUFFER_RID, "Cannot use backbuffer as depth stencil!"); return rtvDsvHandles[dsv]; }
		const HandleSRV& GetSRV(RID srv) const noexcept { ZE_ASSERT(srv < resourceCount, "Resource ID outside available range!"); return srvHandles[srv]; }
		const HandleUAV& GetUAV(RID uav) const noexcept { ZE_ASSERT(uav < resourceCount, "Resource ID outside available range!"); ZE_ASSERT(uav != BACKBUFFER_RID, "Cannot use backbuffer as unnordered access!"); return uavHandles[uav - 1]; }
		
		U64 GetHeapOffset(RID rid, bool tightAlignment) const noexcept;
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
			handles[i] = GetRTV(id);
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
		ZE_ASSERT(GetDSV(dsv).ptr != UINT64_MAX, "Current resource is not suitable for being depth stencil!");
		EnterRaster();

		D3D12_CPU_DESCRIPTOR_HANDLE handles[RTVCount];
		D3D12_VIEWPORT vieports[RTVCount];
		D3D12_RECT scissorRects[RTVCount];
		for (U32 i = 0; i < RTVCount; ++i)
		{
			RID id = rtv[i];
			handles[i] = GetRTV(id);
			ZE_ASSERT(handles[i].ptr != UINT64_MAX, "Current resource is not suitable for being render target!");
			SetupViewport(vieports[i], scissorRects[i], id);
		}
		cl.Get().dx12.GetList()->RSSetViewports(RTVCount, vieports);
		cl.Get().dx12.GetList()->RSSetScissorRects(RTVCount, scissorRects);
		cl.Get().dx12.GetList()->OMSetRenderTargets(RTVCount, handles, adjacent, rtvDsvHandles.get() + dsv);
	}

	template<U32 BarrierCount>
	void FrameBuffer::Barrier(GFX::CommandList& cl, const std::array<GFX::Pipeline::BarrierTransition, BarrierCount>& barriers) const noexcept
	{
		static_assert(BarrierCount > 1, "For performance reasons FrameBuffer::Barrier() should be only used for multiple barriers!");

		D3D12_TEXTURE_BARRIER texBarriers[BarrierCount];
		for (U32 i = 0; i < BarrierCount; ++i)
		{
			ZE_ASSERT(!IsBuffer(barriers.at(i).Resource), "Buffer resources are not supported in this barrier path!");
			FillBarier(texBarriers[i], barriers.at(i));
		}
		PerformBarrier(cl.Get().dx12, texBarriers, BarrierCount, nullptr, 0);
	}
#pragma endregion
}