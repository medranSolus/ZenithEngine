#pragma once
#include "GFX/Context.h"
#include "D3D11.h"

namespace ZE::GFX::API::DX11::Resource
{
	class IndexBuffer final
	{
		DX::ComPtr<ID3D11Buffer> buffer;

	public:
		IndexBuffer(GFX::Device& dev, U32 count, U32* indices);
		IndexBuffer(IndexBuffer&&) = delete;
		IndexBuffer(const IndexBuffer&) = delete;
		IndexBuffer& operator=(IndexBuffer&&) = delete;
		IndexBuffer& operator=(const IndexBuffer&) = delete;
		~IndexBuffer() = default;

		void Bind(GFX::Context& ctx) const noexcept;
		U32* GetData(GFX::Device& dev, GFX::Context& ctx) const;
	};
}