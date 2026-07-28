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

		Status AllocBlock(Device& dev) noexcept;
		Status MapBlock(U64 block) noexcept;

	public:
		DynamicCBuffer() = default;
		ZE_CLASS_NO_COPY(DynamicCBuffer);
		constexpr DynamicCBuffer(DynamicCBuffer&& buff) noexcept;
		constexpr DynamicCBuffer& operator=(DynamicCBuffer&& buff) noexcept;
		~DynamicCBuffer();

		static Expected<DynamicCBuffer> Create(GFX::Device& dev) noexcept;

		Expected<GFX::Resource::DynamicBufferAlloc> Alloc(GFX::Device& dev, const void* values, U32 bytes) noexcept { return Alloc(dev.Get().dx12, values, bytes); }

		void Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, const GFX::Resource::DynamicBufferAlloc& allocInfo) const noexcept;
		Status StartFrame(GFX::Device& dev) noexcept;

		// Gfx API Internal

		IResource* GetAllocBuffer(const GFX::Resource::DynamicBufferAlloc& alloc) const noexcept { return resInfo.at(alloc.Block).first.Resource.Get(); }
		D3D12_GPU_VIRTUAL_ADDRESS GetBindHandle(const GFX::Resource::DynamicBufferAlloc& alloc) const noexcept { return resInfo.at(alloc.Block).second + alloc.Offset; }

		Expected<GFX::Resource::DynamicBufferAlloc> Alloc(Device& dev, const void* values, U32 bytes) noexcept;
	};

#pragma region Functions
	constexpr DynamicCBuffer::DynamicCBuffer(DynamicCBuffer&& buff) noexcept
		: resInfo(std::move(buff.resInfo)), buffer(std::move(buff.buffer)),
		nextOffset(buff.nextOffset), currentBlock(buff.currentBlock)
	{
	}

	constexpr DynamicCBuffer& DynamicCBuffer::operator=(DynamicCBuffer&& buff) noexcept
	{
		std::swap(resInfo, buff.resInfo);
		std::swap(buffer, buff.buffer);
		nextOffset = buff.nextOffset;
		currentBlock = buff.currentBlock;
		return *this;
	}
#pragma endregion
}