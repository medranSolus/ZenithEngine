#include "RHI/DX11/Resource/CBuffer.h"
#include "Data/ResourceLocation.h"

namespace ZE::RHI::DX11::Resource
{
	CBuffer::CBuffer(Device& dev, const void* values, U32 bytes, bool dynamic) : dynamic(dynamic)
	{
		ZE_DX_ENABLE_ID(dev);

		D3D11_BUFFER_DESC bufferDesc = {};
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
		const void* srcData = values;
		std::unique_ptr<U8[]> buffData = nullptr;
		if (bufferDesc.ByteWidth > bytes)
		{
			buffData = std::make_unique<U8[]>(bufferDesc.ByteWidth);
			std::memcpy(buffData.get(), srcData, bytes);
			srcData = buffData.get();
		}

		D3D11_SUBRESOURCE_DATA resData = {};
		resData.pSysMem = srcData;
		resData.SysMemPitch = 0;
		resData.SysMemSlicePitch = 0;

		ZE_DX_THROW_FAILED(dev.GetDevice()->CreateBuffer(&bufferDesc, &resData, &buffer));
		ZE_DX_SET_ID(buffer, "CBuffer");
	}

	CBuffer::CBuffer(GFX::Device& dev, IO::DiskManager& disk, const GFX::Resource::CBufferData& data)
		: CBuffer(dev.Get().dx11, data.DataRef.get() ? data.DataRef.get() : data.DataStatic, data.Bytes, false)
	{
		if (data.ResourceID != INVALID_EID)
			Settings::Data.get<Data::ResourceLocationAtom>(data.ResourceID) = Data::ResourceLocation::GPU;
	}

	CBuffer::CBuffer(GFX::Device& dev, IO::DiskManager& disk, const GFX::Resource::CBufferFileData& data, IO::File& file)
	{
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

	void CBuffer::GetData(GFX::Device& dev, void* values, U32 bytes) const
	{
		ZE_DX_ENABLE_ID(dev.Get().dx11);

		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.Usage = D3D11_USAGE_STAGING;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		bufferDesc.MiscFlags = 0;
		bufferDesc.ByteWidth = Math::AlignUp(bytes, 16U);
		bufferDesc.StructureByteStride = 0;

		DX::ComPtr<IBuffer> stagingBuffer = nullptr;
		ZE_DX_THROW_FAILED(dev.Get().dx11.GetDevice()->CreateBuffer(&bufferDesc, nullptr, &stagingBuffer));
		ZE_DX_SET_ID(buffer, "CBuffer_Staging");

		IDeviceContext* ctx = dev.Get().dx11.GetMainContext();
		ctx->CopyResource(stagingBuffer.Get(), buffer.Get());
		D3D11_MAPPED_SUBRESOURCE subres;
		ZE_DX_THROW_FAILED(ctx->Map(stagingBuffer.Get(), 0, D3D11_MAP_READ, 0, &subres));
		memcpy(values, subres.pData, bytes);
		ctx->Unmap(stagingBuffer.Get(), 0);
	}

	void CBuffer::Update(Device& dev, const GFX::Resource::CBufferData& data) const
	{
		if (dynamic)
		{
			ZE_DX_ENABLE(dev);

			D3D11_MAPPED_SUBRESOURCE subres;
			ZE_DX_THROW_FAILED(dev.GetMainContext()->Map(buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subres));
			std::memcpy(subres.pData, data.DataRef.get() ? data.DataRef.get() : data.DataStatic, data.Bytes);
			dev.GetMainContext()->Unmap(buffer.Get(), 0);
		}
		else
		{
			dev.GetMainContext()->UpdateSubresource(buffer.Get(), 0, nullptr, data.DataRef.get() ? data.DataRef.get() : data.DataStatic, 0, 0);

			if (data.ResourceID != INVALID_EID)
				Settings::Data.get<Data::ResourceLocationAtom>(data.ResourceID) = Data::ResourceLocation::GPU;
		}
	}
}