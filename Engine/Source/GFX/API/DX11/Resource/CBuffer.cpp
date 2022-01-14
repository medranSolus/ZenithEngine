#include "GFX/API/DX11/Resource/CBuffer.h"

namespace ZE::GFX::API::DX11::Resource
{
	CBuffer::CBuffer(GFX::Device& dev, const U8* values, U32 bytes, bool dynamic)
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
	}

	void CBuffer::Update(GFX::CommandList& cl, const U8* values, U32 bytes) const
	{
		cl.Get().dx11.GetContext()->UpdateSubresource(buffer.Get(), 0, nullptr, values, 0, 0);
	}

	void CBuffer::UpdateDynamic(GFX::Device& dev, GFX::CommandList& cl, const U8* values, U32 bytes) const
	{
		ZE_GFX_ENABLE(dev.Get().dx11);

		D3D11_MAPPED_SUBRESOURCE subres;
		ZE_GFX_THROW_FAILED(cl.Get().dx11.GetContext()->Map(buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subres));
		memcpy(subres.pData, values, bytes);
		cl.Get().dx11.GetContext()->Unmap(buffer.Get(), 0);
	}

	void CBuffer::BindVS(GFX::CommandList& cl, ShaderSlot slot) const noexcept
	{
		assert(slot < D3D11_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT);
		cl.Get().dx11.GetContext()->VSSetConstantBuffers(slot, 1, buffer.GetAddressOf());
	}

	void CBuffer::BindDS(GFX::CommandList& cl, ShaderSlot slot) const noexcept
	{
		assert(slot < D3D11_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT);
		cl.Get().dx11.GetContext()->DSSetConstantBuffers(slot, 1, buffer.GetAddressOf());
	}

	void CBuffer::BindHS(GFX::CommandList& cl, ShaderSlot slot) const noexcept
	{
		assert(slot < D3D11_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT);
		cl.Get().dx11.GetContext()->HSSetConstantBuffers(slot, 1, buffer.GetAddressOf());
	}

	void CBuffer::BindGS(GFX::CommandList& cl, ShaderSlot slot) const noexcept
	{
		assert(slot < D3D11_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT);
		cl.Get().dx11.GetContext()->GSSetConstantBuffers(slot, 1, buffer.GetAddressOf());
	}

	void CBuffer::BindPS(GFX::CommandList& cl, ShaderSlot slot) const noexcept
	{
		assert(slot < D3D11_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT);
		cl.Get().dx11.GetContext()->PSSetConstantBuffers(slot, 1, buffer.GetAddressOf());
	}

	void CBuffer::BindCS(GFX::CommandList& cl, ShaderSlot slot) const noexcept
	{
		assert(slot < D3D11_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT);
		cl.Get().dx11.GetContext()->CSSetConstantBuffers(slot, 1, buffer.GetAddressOf());
	}
}