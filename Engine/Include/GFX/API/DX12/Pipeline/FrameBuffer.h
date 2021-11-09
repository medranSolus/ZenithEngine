#pragma once
#include "GFX/Pipeline/FrameBufferDesc.h"
#include "GFX/SwapChain.h"
#include "D3D12.h"

namespace ZE::GFX::API::DX12::Pipeline
{
	class FrameBuffer final
	{
		DX::ComPtr<ID3D12Resource>* resources;
		D3D12_CPU_DESCRIPTOR_HANDLE* rtvDsv;
		D3D12_CPU_DESCRIPTOR_HANDLE* srv;
		std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE>* uav;
		DX::ComPtr<ID3D12DescriptorHeap> rtvDescHeap;
		DX::ComPtr<ID3D12DescriptorHeap> dsvDescHeap;
		DX::ComPtr<ID3D12Heap> mainHeap;
		DX::ComPtr<ID3D12Heap> uavHeap = nullptr;

#ifdef _ZE_DEBUG_FRAME_MEMORY_PRINT
		static void PrintMemory(std::string&& memID, U32 maxChunks, U64 levelCount, U64 invalidID, const std::vector<U64>& memory);
#endif
		static U64 FindHeapSize(U32 maxChunks, U64 levelCount, U64 invalidID, const std::vector<U64>& memory) noexcept;
		static U32 AllocResource(U64 id, U32 chunks, U64 startLevel, U64 lastLevel, U32 maxChunks, U64 levelCount, U64 invalidID, std::vector<U64>& memory);

	public:
		FrameBuffer(GFX::Device& dev, GFX::SwapChain& swapChain,
			GFX::Pipeline::FrameBufferDesc& desc);
		FrameBuffer(FrameBuffer&&) = default;
		FrameBuffer(const FrameBuffer&) = delete;
		FrameBuffer& operator=(FrameBuffer&&) = default;
		FrameBuffer& operator=(const FrameBuffer&) = delete;
		~FrameBuffer();
	};
}