#pragma once
#include "GFX/Device.h"
#include "GFX/IndexData.h"
#include "D3D11.h"

namespace ZE::GFX::API::DX11::Resource
{
	class IndexBuffer final
	{
		DX::ComPtr<ID3D11Buffer> buffer;
		U32 count;

	public:
		IndexBuffer(GFX::Device& dev, const IndexData& data);
		ZE_CLASS_MOVE(IndexBuffer);
		~IndexBuffer() = default;

		constexpr U32 GetCount() const noexcept { return count; }
		void Free(GFX::Device& dev) noexcept { buffer = nullptr; }

		void Bind(GFX::CommandList& cl) const noexcept;
		IndexData GetData(GFX::Device& dev, GFX::CommandList& cl) const;
	};
}