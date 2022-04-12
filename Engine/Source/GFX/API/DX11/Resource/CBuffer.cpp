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
		bufferDesc.ByteWidth = Math::AlignUp(bytes, 16U);
		bufferDesc.StructureByteStride = 0;

		// If aligned size is greater than actual data then create new region
		const void* data = values;
		if (bufferDesc.ByteWidth > bytes)
		{
			U8* buffData = new U8[bufferDesc.ByteWidth];
			memcpy(buffData, values, bytes);
			data = buffData;
		}

		D3D11_SUBRESOURCE_DATA resData;
		resData.pSysMem = data;
		resData.SysMemPitch = 0;
		resData.SysMemSlicePitch = 0;

		ZE_GFX_THROW_FAILED(dev.Get().dx11.GetDevice()->CreateBuffer(&bufferDesc, values ? &resData : nullptr, &buffer));
		ZE_GFX_SET_ID(buffer, "CBuffer");

		if (bufferDesc.ByteWidth > bytes)
			delete[] reinterpret_cast<const U8*>(data);
		if (dynamic)
		{
			D3D11_MAPPED_SUBRESOURCE subres;
			ZE_GFX_THROW_FAILED(dev.Get().dx11.GetMainContext()->Map(buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subres));
			bufferData = subres.pData;
		}
	}

	void CBuffer::Update(GFX::Device& dev, const void* values, U32 bytes) const
	{
		if (bufferData)
			memcpy(bufferData, values, bytes);
		else
			dev.Get().dx11.GetMainContext()->UpdateSubresource(buffer.Get(), 0, nullptr, values, 0, 0);
	}

	void CBuffer::Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept
	{
		auto slotInfo = bindCtx.BindingSchema.Get().dx11.GetCurrentSlot(bindCtx.Count++);

		if (slotInfo.first & GFX::Resource::ShaderType::Compute)
			cl.Get().dx11.GetContext()->CSSetConstantBuffers(slotInfo.second, 1, buffer.GetAddressOf());
		else
		{
			if (slotInfo.first & GFX::Resource::ShaderType::Vertex)
				cl.Get().dx11.GetContext()->VSSetConstantBuffers(slotInfo.second, 1, buffer.GetAddressOf());
			if (slotInfo.first & GFX::Resource::ShaderType::Domain)
				cl.Get().dx11.GetContext()->DSSetConstantBuffers(slotInfo.second, 1, buffer.GetAddressOf());
			if (slotInfo.first & GFX::Resource::ShaderType::Hull)
				cl.Get().dx11.GetContext()->HSSetConstantBuffers(slotInfo.second, 1, buffer.GetAddressOf());
			if (slotInfo.first & GFX::Resource::ShaderType::Geometry)
				cl.Get().dx11.GetContext()->GSSetConstantBuffers(slotInfo.second, 1, buffer.GetAddressOf());
			if (slotInfo.first & GFX::Resource::ShaderType::Pixel)
				cl.Get().dx11.GetContext()->PSSetConstantBuffers(slotInfo.second, 1, buffer.GetAddressOf());
		}
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