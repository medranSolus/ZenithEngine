#pragma once
#include "GFX/IndexData.h"
#include "GFX/Device.h"

namespace ZE::GFX::API::DX11::Resource
{
	class IndexBuffer final
	{
		DX::ComPtr<IBuffer> buffer;
		U32 count;

	public:
		IndexBuffer() = default;
		IndexBuffer(GFX::Device& dev, const IndexData& data);
		ZE_CLASS_MOVE(IndexBuffer);
		~IndexBuffer() = default;

		constexpr U32 GetCount() const noexcept { return count; }
		void Free(GFX::Device& dev) noexcept { count = 0; buffer = nullptr; }

		void Bind(GFX::CommandList& cl) const noexcept;
		IndexData GetData(GFX::Device& dev, GFX::CommandList& cl) const;
	};
}