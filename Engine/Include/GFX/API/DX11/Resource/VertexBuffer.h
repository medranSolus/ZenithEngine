#pragma once
#include "GFX/Device.h"
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
		ZE_CLASS_MOVE(VertexBuffer);
		~VertexBuffer() = default;

		void Bind(GFX::CommandList& cl) const noexcept;
		VertexData GetData(GFX::Device& dev, GFX::CommandList& cl) const;
	};
}