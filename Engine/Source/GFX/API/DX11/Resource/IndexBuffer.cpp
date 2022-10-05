#include "GFX/API/DX11/Resource/IndexBuffer.h"

namespace ZE::GFX::API::DX11::Resource
{
	IndexBuffer::IndexBuffer(GFX::Device& dev, const IndexData& data) : count(data.Count)
	{
		assert(data.Indices != nullptr && data.Count != 0 && data.Count % 3 == 0);
		ZE_DX_ENABLE_ID(dev.Get().dx11);

		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.ByteWidth = static_cast<UINT>(count * sizeof(U32));
		bufferDesc.StructureByteStride = sizeof(U32);

		D3D11_SUBRESOURCE_DATA resData;
		resData.pSysMem = data.Indices;
		resData.SysMemPitch = 0;
		resData.SysMemSlicePitch = 0;

		ZE_DX_THROW_FAILED(dev.Get().dx11.GetDevice()->CreateBuffer(&bufferDesc, &resData, &buffer));
		ZE_DX_SET_ID(buffer, "IndexBuffer");
	}

	void IndexBuffer::Bind(GFX::CommandList& cl) const noexcept
	{
		cl.Get().dx11.GetContext()->IASetIndexBuffer(buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	}

	IndexData IndexBuffer::GetData(GFX::Device& dev, GFX::CommandList& cl) const
	{
		return { 0, nullptr };
	}
}