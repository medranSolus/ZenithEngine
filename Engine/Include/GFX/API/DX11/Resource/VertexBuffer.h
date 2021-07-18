#pragma once
#include "GFX/Context.h"
#include "GFX/VertexData.h"
#include "D3D11.h"

namespace ZE::GFX::API::DX11::Resource
{
	class VertexBuffer final
	{
		UINT byteStride;
		DX::ComPtr<ID3D11Buffer> buffer;

	public:
		VertexBuffer(GFX::Device& dev, const VertexData& data);
		VertexBuffer(VertexBuffer&&) = delete;
		VertexBuffer(const VertexBuffer&) = delete;
		VertexBuffer& operator=(VertexBuffer&&) = delete;
		VertexBuffer& operator=(const VertexBuffer&) = delete;
		~VertexBuffer() = default;

		void Bind(GFX::Context& ctx) const noexcept;
		VertexData GetData(GFX::Device& dev, GFX::Context& ctx) const;
	};
}