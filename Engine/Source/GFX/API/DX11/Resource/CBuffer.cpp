#include "GFX/API/DX11/Resource/CBuffer.h"

namespace ZE::GFX::API::DX11::Resource
{
	CBuffer::CBuffer(GFX::Device& dev, const void* values, U32 bytes, bool dynamic) : dynamic(dynamic)
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
	}

	void* CBuffer::GetRegion(GFX::Device& dev) const
	{
		ZE_ASSERT(dynamic, "CBuffer is not dynamic!");
		ZE_GFX_ENABLE(dev.Get().dx11);

		D3D11_MAPPED_SUBRESOURCE subres;
		ZE_GFX_THROW_FAILED(dev.Get().dx11.GetMainContext()->Map(buffer.Get(), 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &subres));
		return subres.pData;
	}

	void CBuffer::FlushRegion(GFX::Device& dev) const noexcept
	{
		ZE_ASSERT(dynamic, "CBuffer is not dynamic!");
		dev.Get().dx11.GetMainContext()->Unmap(buffer.Get(), 0);
	}

	void CBuffer::Update(GFX::Device& dev, const void* values, U32 bytes) const
	{
		if (dynamic)
		{
			ZE_GFX_ENABLE(dev.Get().dx11);

			D3D11_MAPPED_SUBRESOURCE subres;
			ZE_GFX_THROW_FAILED(dev.Get().dx11.GetMainContext()->Map(buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subres));
			memcpy(subres.pData, values, bytes);
			dev.Get().dx11.GetMainContext()->Unmap(buffer.Get(), 0);
		}
		else
			dev.Get().dx11.GetMainContext()->UpdateSubresource(buffer.Get(), 0, nullptr, values, 0, 0);
	}

	void CBuffer::Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept
	{
		auto& schema = bindCtx.BindingSchema.Get().dx11;

		auto slotInfo = schema.GetCurrentSlot(bindCtx.Count++);
		ZE_ASSERT(slotInfo.SlotsCount == 1, "Constant buffer slot should only contain 1 entry!");

		auto slotData = schema.GetSlotData(slotInfo.DataStart);
		ZE_ASSERT(slotData.Count == 1, "Constant buffer slot should only be bound as single buffer!");

		auto* ctx = cl.Get().dx11.GetContext();
		if (slotData.Shaders & GFX::Resource::ShaderType::Compute)
			ctx->CSSetConstantBuffers(slotData.BindStart, 1, buffer.GetAddressOf());
		else
		{
			if (slotData.Shaders & GFX::Resource::ShaderType::Vertex)
				ctx->VSSetConstantBuffers(slotData.BindStart, 1, buffer.GetAddressOf());
			if (slotData.Shaders & GFX::Resource::ShaderType::Domain)
				ctx->DSSetConstantBuffers(slotData.BindStart, 1, buffer.GetAddressOf());
			if (slotData.Shaders & GFX::Resource::ShaderType::Hull)
				ctx->HSSetConstantBuffers(slotData.BindStart, 1, buffer.GetAddressOf());
			if (slotData.Shaders & GFX::Resource::ShaderType::Geometry)
				ctx->GSSetConstantBuffers(slotData.BindStart, 1, buffer.GetAddressOf());
			if (slotData.Shaders & GFX::Resource::ShaderType::Pixel)
				ctx->PSSetConstantBuffers(slotData.BindStart, 1, buffer.GetAddressOf());
		}
	}

	void CBuffer::Free(GFX::Device& dev) noexcept
	{
		buffer = nullptr;
	}
}