#include "GFX/API/DX11/Resource/CBuffer.h"

namespace ZE::GFX::API::DX11::Resource
{
	CBuffer::CBuffer(GFX::Device& dev, U8* values, U32 bytes, bool dynamic)
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

	void CBuffer::BindVS(GFX::Context& ctx, ShaderSlot slot) const noexcept
	{
		assert(slot < D3D11_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT);
		ctx.Get().dx11.GetContext()->VSSetConstantBuffers(slot, 1, buffer.GetAddressOf());
	}

	void CBuffer::BindDS(GFX::Context& ctx, ShaderSlot slot) const noexcept
	{
		assert(slot < D3D11_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT);
		ctx.Get().dx11.GetContext()->DSSetConstantBuffers(slot, 1, buffer.GetAddressOf());
	}

	void CBuffer::BindHS(GFX::Context& ctx, ShaderSlot slot) const noexcept
	{
		assert(slot < D3D11_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT);
		ctx.Get().dx11.GetContext()->HSSetConstantBuffers(slot, 1, buffer.GetAddressOf());
	}

	void CBuffer::BindGS(GFX::Context& ctx, ShaderSlot slot) const noexcept
	{
		assert(slot < D3D11_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT);
		ctx.Get().dx11.GetContext()->GSSetConstantBuffers(slot, 1, buffer.GetAddressOf());
	}

	void CBuffer::BindPS(GFX::Context& ctx, ShaderSlot slot) const noexcept
	{
		assert(slot < D3D11_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT);
		ctx.Get().dx11.GetContext()->PSSetConstantBuffers(slot, 1, buffer.GetAddressOf());
	}

	void CBuffer::BindCS(GFX::Context& ctx, ShaderSlot slot) const noexcept
	{
		assert(slot < D3D11_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT);
		ctx.Get().dx11.GetContext()->CSSetConstantBuffers(slot, 1, buffer.GetAddressOf());
	}
}