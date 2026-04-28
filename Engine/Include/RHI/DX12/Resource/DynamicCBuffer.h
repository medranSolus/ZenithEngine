#pragma once
#include "GFX/Resource/DynamicBufferAlloc.h"
#include "GFX/Binding/Context.h"
#include "GFX/CommandList.h"

namespace ZE::RHI::DX12::Resource
{
	class DynamicCBuffer final
	{
		static constexpr U64 BLOCK_SHRINK_STEP = 2;

		std::vector<std::pair<ResourceInfo, D3D12_GPU_VIRTUAL_ADDRESS>> resInfo;
		Ptr<U8> buffer = nullptr;
		U32 nextOffset = 0;
		U64 currentBlock = 0;
#if !_ZE_RENDER_GRAPH_SINGLE_THREAD
		std::mutex allocLock;
#endif
		Device* srcDev = nullptr;

		Status AllocBlock(GFX::Device& dev) noexcept;
		Status MapBlock(GFX::Device& dev, U64 block) noexcept;

	public:
		DynamicCBuffer() = default;
		ZE_CLASS_MOVE(DynamicCBuffer);
		~DynamicCBuffer();

		static Expected<DynamicCBuffer> Create(GFX::Device& dev) noexcept;

		Expected<GFX::Resource::DynamicBufferAlloc> Alloc(GFX::Device& dev, const void* values, U32 bytes) noexcept;
		void Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, const GFX::Resource::DynamicBufferAlloc& allocInfo) const noexcept;
		Status StartFrame(GFX::Device& dev) noexcept;
	};
}