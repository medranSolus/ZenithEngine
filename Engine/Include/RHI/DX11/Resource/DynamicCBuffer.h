#pragma once
#include "GFX/Binding/Context.h"
#include "GFX/Resource/DynamicBufferAlloc.h"
#include "Data/Library.h"
#include "GFX/CommandList.h"

namespace ZE::RHI::DX11::Resource
{
	class DynamicCBuffer final
	{
		static constexpr U64 BLOCK_SHRINK_STEP = 2;
		static constexpr U32 BLOCK_SIZE = 64 * Math::KILOBYTE;

		std::vector<std::pair<DX::ComPtr<IBuffer>, Data::Library<U32, U32>>> blocks;
		U32 nextOffset = 0;
		U64 currentBlock = 0;
#ifndef _ZE_RENDER_GRAPH_SINGLE_THREAD
		std::mutex allocLock;
#endif

		Status AllocBlock(GFX::Device& dev) noexcept;

	public:
		DynamicCBuffer() = default;
		ZE_CLASS_MOVE(DynamicCBuffer);
		~DynamicCBuffer() = default;

		static Expected<DynamicCBuffer> Create(GFX::Device& dev) noexcept;

		Expected<GFX::Resource::DynamicBufferAlloc> Alloc(GFX::Device& dev, const void* values, U32 bytes) noexcept;
		void Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, const GFX::Resource::DynamicBufferAlloc& allocInfo) const noexcept;
		Status StartFrame(GFX::Device& dev) noexcept;
	};
}