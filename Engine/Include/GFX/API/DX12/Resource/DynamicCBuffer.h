#pragma once
#include "GFX/Device.h"
#include "GFX/Binding/Context.h"
#include "GFX/Resource/DynamicBufferAlloc.h"
#include "D3D12.h"
#include <mutex>

namespace ZE::GFX::API::DX12::Resource
{
	class DynamicCBuffer final
	{
		static constexpr U64 BLOCK_SHRINK_STEP = 2;

		std::vector<std::pair<ResourceInfo, D3D12_GPU_VIRTUAL_ADDRESS>> resInfo;
		U8* buffer;
		U32 nextOffset = 0;
		U64 currentBlock = 0;
#ifndef _ZE_RENDER_GRAPH_SINGLE_THREAD
		std::mutex allocLock;
#endif

		void AllocBlock(GFX::Device& dev);
		void MapBlock(GFX::Device& dev, U64 block);

	public:
		DynamicCBuffer() = default;
		DynamicCBuffer(GFX::Device& dev) { AllocBlock(dev); MapBlock(dev, 0); }
		ZE_CLASS_MOVE(DynamicCBuffer);
		~DynamicCBuffer() = default;

		GFX::Resource::DynamicBufferAlloc Alloc(GFX::Device& dev, const void* values, U32 bytes);
		void Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, const GFX::Resource::DynamicBufferAlloc& allocInfo) const noexcept;
		void StartFrame(GFX::Device& dev);
		void Free(GFX::Device& dev) noexcept;
	};
}