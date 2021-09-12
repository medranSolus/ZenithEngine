#pragma once
#include "GFX/CommandList.h"
#include "D3D11.h"

namespace ZE::GFX::API::DX11::Resource
{
	class IndexBuffer final
	{
		DX::ComPtr<ID3D11Buffer> buffer;

	public:
		IndexBuffer(GFX::Device& dev, U32 count, U32* indices);
		IndexBuffer(IndexBuffer&&) = default;
		IndexBuffer(const IndexBuffer&) = delete;
		IndexBuffer& operator=(IndexBuffer&&) = default;
		IndexBuffer& operator=(const IndexBuffer&) = delete;
		~IndexBuffer() = default;

		void Free(GFX::Device& dev) noexcept { buffer = nullptr; }

		void Bind(GFX::CommandList& cl) const noexcept;
		U32* GetData(GFX::Device& dev, GFX::CommandList& cl) const;
	};
}