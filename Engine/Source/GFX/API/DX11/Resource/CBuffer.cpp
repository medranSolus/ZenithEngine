#include "GFX/API/DX11/Resource/CBuffer.h"

namespace ZE::GFX::API::DX11::Resource
{
	CBuffer::CBuffer(GFX::Device& dev, const void* values, U32 bytes, bool dynamic)
	{
		ZE_GFX_ENABLE_ID(dev.Get().dx11);

		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		if (dynamic)
		{
			bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
			bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		}
		else
		{
			bufferDesc.Usage = D3D11_USAGE_DEFAULT;
			bufferDesc.CPUAccessFlags = 0;
		}
		bufferDesc.MiscFlags = 0;
		bufferDesc.ByteWidth = bytes;
		bufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA resData;
		resData.pSysMem = values;
		resData.SysMemPitch = 0;
		resData.SysMemSlicePitch = 0;

		ZE_GFX_THROW_FAILED(dev.Get().dx11.GetDevice()->CreateBuffer(&bufferDesc, &resData, &buffer));
		ZE_GFX_SET_ID(buffer, "CBuffer");

		if (dynamic)
		{
			D3D11_MAPPED_SUBRESOURCE subres;
			ZE_GFX_THROW_FAILED(dev.Get().dx11.GetMainContext()->Map(buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subres));
			bufferData = subres.pData;
		}
	}

	void CBuffer::Update(GFX::Device& dev, GFX::CommandList& cl, const void* values, U32 bytes) const
	{
		if (bufferData)
			memcpy(bufferData, values, bytes);
		else
			cl.Get().dx11.GetContext()->UpdateSubresource(buffer.Get(), 0, nullptr, values, 0, 0);
	}

	void CBuffer::Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept
	{
		//assert(slot < D3D11_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT);
		//cl.Get().dx11.GetContext()->VSSetConstantBuffers(slot, 1, buffer.GetAddressOf());
	}

	void CBuffer::Free(GFX::Device& dev) noexcept
	{
		if (bufferData)
		{
			bufferData = nullptr;
			dev.Get().dx11.GetMainContext()->Unmap(buffer.Get(), 0);
		}
		buffer = nullptr;
	}
}