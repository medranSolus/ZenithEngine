#pragma once
#include "GFX/CommandList.h"
#include "GFX/IndexData.h"

namespace ZE::RHI::DX11::Resource
{
	class IndexBuffer final
	{
		DX::ComPtr<IBuffer> buffer;
		U32 count;
		bool is16bit;

	public:
		IndexBuffer() = default;
		IndexBuffer(GFX::Device& dev, const GFX::IndexData& data);
		ZE_CLASS_MOVE(IndexBuffer);
		~IndexBuffer() { ZE_ASSERT(count == 0 && buffer == nullptr, "Resource not freed before deletion!"); }

		constexpr U32 GetCount() const noexcept { return count; }
		void Free(GFX::Device& dev) noexcept { count = 0; buffer = nullptr; }

		void Bind(GFX::CommandList& cl) const noexcept;
		GFX::IndexData GetData(GFX::Device& dev, GFX::CommandList& cl) const;
	};
}