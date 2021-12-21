#pragma once
#include "GFX/Pipeline/FrameBufferDesc.h"
#include "GFX/Pipeline/SyncType.h"
#include "GFX/CommandList.h"
#include "GFX/SwapChain.h"
#include "D3D12.h"

namespace ZE::GFX::API::DX12::Pipeline
{
	class FrameBuffer final
	{
		struct TransitionPoint
		{
			U32 BarrierCount = 0;
			D3D12_RESOURCE_BARRIER* Barriers = nullptr;
			GFX::Pipeline::SyncType AfterSync = GFX::Pipeline::SyncType::None;
		};

		TransitionPoint initTransitions;
		TransitionPoint* transitions = nullptr;

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
			RID invalidID, const std::vector<RID>& memory, U64 heapSize);
#endif
		static U64 FindHeapSize(U32 maxChunks, U64 levelCount, RID invalidID, const std::vector<RID>& memory) noexcept;
		static bool CheckResourceAliasing(U32 offset, U32 chunks, U64 startLevel, U64 lastLevel,
			U32 maxChunks, U64 levelCount, RID invalidID, const std::vector<RID>& memory) noexcept;
		static U32 AllocResource(RID id, U32 chunks, U64 startLevel, U64 lastLevel,
			U32 maxChunks, U64 levelCount, RID invalidID, std::vector<RID>& memory);

		void InitResource(CommandList& cl, RID rid) const noexcept;

	public:
		FrameBuffer(GFX::Device& dev, GFX::CommandList& mainList, GFX::Pipeline::FrameBufferDesc& desc);
		ZE_CLASS_DELETE(FrameBuffer);
		~FrameBuffer();

		void InitRTV(GFX::CommandList& cl, RID rid) const noexcept { InitResource(cl.Get().dx12, rid); }
		void InitDSV(GFX::CommandList& cl, U64 rid) const noexcept { InitResource(cl.Get().dx12, rid); }

		void ClearRTV(GFX::Device& dev, GFX::CommandList& cl, RID rid, const ColorF4 color) const;
		void ClearDSV(GFX::Device& dev, GFX::CommandList& cl, RID rid, float depth, U8 stencil) const;

		void SwapBackbuffer(GFX::Device& dev, GFX::SwapChain& swapChain);
		void InitTransitions(GFX::Device& dev, GFX::CommandList& cl) const;
		void ExitTransitions(GFX::Device& dev, GFX::CommandList& cl, U64 level) const noexcept;
	};
}