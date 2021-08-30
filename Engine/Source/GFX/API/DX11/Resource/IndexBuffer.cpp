#include "GFX/API/DX11/Resource/IndexBuffer.h"
#include "GFX/API/DX/GraphicsException.h"

namespace ZE::GFX::API::DX11::Resource
{
	IndexBuffer::IndexBuffer(GFX::Device& dev, U32 count, U32* indices)
	{
		ZE_GFX_ENABLE_ID(dev.Get().dx11);

		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.ByteWidth = static_cast<UINT>(count * sizeof(U32));
		bufferDesc.StructureByteStride = sizeof(U32);

		D3D11_SUBRESOURCE_DATA resData;
		resData.pSysMem = indices;
		resData.SysMemPitch = 0;
		resData.SysMemSlicePitch = 0;

		ZE_GFX_THROW_FAILED(dev.Get().dx11.GetDevice()->CreateBuffer(&bufferDesc, &resData, &buffer));
		ZE_GFX_SET_ID(buffer, "IndexBuffer");
	}

	void IndexBuffer::Bind(GFX::CommandList& cl) const noexcept
	{
		cl.Get().dx11.GetContext()->IASetIndexBuffer(buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	}

	U32* IndexBuffer::GetData(GFX::Device& dev, GFX::CommandList& cl) const
	{
		return nullptr;
	}
}