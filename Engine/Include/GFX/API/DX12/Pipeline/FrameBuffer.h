#pragma once
#include "GFX/Pipeline/FrameBufferDesc.h"
#include "GFX/CommandList.h"
#include "GFX/SwapChain.h"
#include "D3D12.h"

namespace ZE::GFX::API::DX12::Pipeline
{
	class FrameBuffer final
	{
		std::pair<U32, D3D12_RESOURCE_BARRIER*> initBarriers;
		std::pair<U32, D3D12_RESOURCE_BARRIER*>* barriers = nullptr;

		U64 backbufferBarriersLocationsCount = 0;
		U64* backbufferBarriersLocations = nullptr;
		bool* aliasingResources = nullptr;
		DX::ComPtr<ID3D12Resource>* resources = nullptr;

		std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE> backbufferRtvSrv;
		D3D12_CPU_DESCRIPTOR_HANDLE* rtvDsv = nullptr;
		D3D12_CPU_DESCRIPTOR_HANDLE* srv = nullptr;
		std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE>* uav = nullptr;

		DX::ComPtr<ID3D12DescriptorHeap> rtvDescHeap;
		DX::ComPtr<ID3D12DescriptorHeap> dsvDescHeap;
		DX::ComPtr<ID3D12Heap> mainHeap;
		DX::ComPtr<ID3D12Heap> uavHeap;

#ifdef _ZE_DEBUG_FRAME_MEMORY_PRINT
		static void PrintMemory(std::string&& memID, U32 maxChunks, U64 levelCount,
			U64 invalidID, const std::vector<U64>& memory, U64 heapSize);
#endif
		static U64 FindHeapSize(U32 maxChunks, U64 levelCount, U64 invalidID, const std::vector<U64>& memory) noexcept;
		static bool CheckResourceAliasing(U32 offset, U32 chunks, U64 startLevel, U64 lastLevel,
			U32 maxChunks, U64 levelCount, U64 invalidID, const std::vector<U64>& memory) noexcept;
		static U32 AllocResource(U64 id, U32 chunks, U64 startLevel, U64 lastLevel,
			U32 maxChunks, U64 levelCount, U64 invalidID, std::vector<U64>& memory);

		void InitResource(CommandList& cl, U64 rid) const noexcept;

	public:
		FrameBuffer(GFX::Device& dev, GFX::CommandList& mainList, GFX::Pipeline::FrameBufferDesc& desc);
		FrameBuffer(FrameBuffer&&) = default;
		FrameBuffer(const FrameBuffer&) = delete;
		FrameBuffer& operator=(FrameBuffer&&) = default;
		FrameBuffer& operator=(const FrameBuffer&) = delete;
		~FrameBuffer();

		void InitRTV(GFX::CommandList& cl, U64 rid) const noexcept { InitResource(cl.Get().dx12, rid); }
		void InitDSV(GFX::CommandList& cl, U64 rid) const noexcept { InitResource(cl.Get().dx12, rid); }

		void ClearRTV(GFX::Device& dev, GFX::CommandList& cl, U64 rid, const ColorF4 color) const;
		void ClearDSV(GFX::Device& dev, GFX::CommandList& cl, U64 rid, float depth, U8 stencil) const;

		void SwapBackbuffer(GFX::Device& dev, GFX::SwapChain& swapChain);
		void InitTransitions(GFX::Device& dev, GFX::CommandList& cl) const;
		void ExitTransitions(U64 level, GFX::CommandList& cl) const noexcept;
	};
}