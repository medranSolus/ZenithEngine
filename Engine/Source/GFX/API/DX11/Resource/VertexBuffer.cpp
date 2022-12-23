#include "GFX/API/DX11/Resource/VertexBuffer.h"

namespace ZE::GFX::API::DX11::Resource
{
	VertexBuffer::VertexBuffer(GFX::Device& dev, const VertexData& data)
		: byteStride(data.VertexSize)
	{
		ZE_DX_ENABLE_ID(dev.Get().dx11);

		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.ByteWidth = data.Count * data.VertexSize;
		bufferDesc.StructureByteStride = byteStride;

		D3D11_SUBRESOURCE_DATA resData;
		resData.pSysMem = data.Vertices;
		resData.SysMemPitch = 0;
		resData.SysMemSlicePitch = 0;

		ZE_DX_THROW_FAILED(dev.Get().dx11.GetDevice()->CreateBuffer(&bufferDesc, &resData, &buffer));
		ZE_DX_SET_ID(buffer, "VertexBuffer");
	}

	void VertexBuffer::Bind(GFX::CommandList& cl) const noexcept
	{
		const UINT offset = 0;
		cl.Get().dx11.GetContext()->IASetVertexBuffers(0, 1, buffer.GetAddressOf(), &byteStride, &offset);
	}

	VertexData VertexBuffer::GetData(GFX::Device& dev, GFX::CommandList& cl) const
	{
		return {};
	}
}