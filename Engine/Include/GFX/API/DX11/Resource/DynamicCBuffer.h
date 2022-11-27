#pragma once
#include "GFX/Binding/Context.h"
#include "GFX/Resource/DynamicBufferAlloc.h"
#include "Data/Library.h"
#include "GFX/Device.h"

namespace ZE::GFX::API::DX11::Resource
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

		void AllocBlock(GFX::Device& dev);

	public:
		DynamicCBuffer() = default;
		DynamicCBuffer(GFX::Device& dev) { AllocBlock(dev); }
		ZE_CLASS_MOVE(DynamicCBuffer);
		~DynamicCBuffer() = default;

		void Free(GFX::Device& dev) noexcept { blocks.clear(); }

		GFX::Resource::DynamicBufferAlloc Alloc(GFX::Device& dev, const void* values, U32 bytes);
		void Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, const GFX::Resource::DynamicBufferAlloc& allocInfo) const noexcept;
		void StartFrame(GFX::Device& dev);
	};
}