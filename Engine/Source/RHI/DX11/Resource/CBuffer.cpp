#include "RHI/DX11/Resource/CBuffer.h"

namespace ZE::RHI::DX11::Resource
{
	Expected<DX::ComPtr<IBuffer>> CreateCBuffer(D3D11_USAGE usage, U32 cpuAccess, Device& dev, const void* data, U32 bytes) noexcept
	{
		ZE_ASSERT(bytes <= D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT, "Trying to create too large CBuffer!");

		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.Usage = usage;
		bufferDesc.CPUAccessFlags = cpuAccess;
		bufferDesc.MiscFlags = 0;
		bufferDesc.ByteWidth = Math::AlignUp(bytes, 16U);
		bufferDesc.StructureByteStride = 0;

		// If aligned size is greater than actual data then create new region
		const void* srcData = data;
		std::unique_ptr<U8[]> buffData = nullptr;
		if (srcData && bufferDesc.ByteWidth > bytes)
		{
			buffData = std::make_unique<U8[]>(bufferDesc.ByteWidth);
			std::memcpy(buffData.get(), data, bytes);
			srcData = buffData.get();
		}

		D3D11_SUBRESOURCE_DATA resData = {};
		resData.pSysMem = srcData;
		resData.SysMemPitch = 0;
		resData.SysMemSlicePitch = 0;

		DX::ComPtr<IBuffer> buffer;
		ZE_DX_RET_FAILED_EXPECT(dev.GetDevice()->CreateBuffer(&bufferDesc, srcData ? &resData : nullptr, &buffer));
		ZE_DX_SET_ID(buffer, "CBuffer");

		return buffer;
	}

	void BindCBuffer(ID3D11Buffer* const* buffer, GFX::CommandList& cl, GFX::Binding::Context& bindCtx) noexcept
	{
		auto& schema = bindCtx.BindingSchema.Get().dx11;

		auto slotInfo = schema.GetCurrentSlot(bindCtx.Count++);
		ZE_ASSERT(slotInfo.SlotsCount == 1, "Constant buffer slot should only contain 1 entry!");

		auto slotData = schema.GetSlotData(slotInfo.DataStart);
		ZE_ASSERT(slotData.Count == 1, "Constant buffer slot should only be bound as single buffer!");

		auto* ctx = cl.Get().dx11.GetContext();
		if (slotData.Shaders & GFX::Resource::ShaderType::Compute)
			ctx->CSSetConstantBuffers(slotData.BindStart, 1, buffer);
		else
		{
			if (slotData.Shaders & GFX::Resource::ShaderType::Vertex)
				ctx->VSSetConstantBuffers(slotData.BindStart, 1, buffer);
			if (slotData.Shaders & GFX::Resource::ShaderType::Domain)
				ctx->DSSetConstantBuffers(slotData.BindStart, 1, buffer);
			if (slotData.Shaders & GFX::Resource::ShaderType::Hull)
				ctx->HSSetConstantBuffers(slotData.BindStart, 1, buffer);
			if (slotData.Shaders & GFX::Resource::ShaderType::Geometry)
				ctx->GSSetConstantBuffers(slotData.BindStart, 1, buffer);
			if (slotData.Shaders & GFX::Resource::ShaderType::Pixel)
				ctx->PSSetConstantBuffers(slotData.BindStart, 1, buffer);
		}
	}

	Expected<CBuffer> CBuffer::Create(GFX::Device& dev, GFX::DiskManager& disk, const GFX::Resource::CBufferData& data) noexcept
	{
		CBuffer cbuff = {};
		ZE_EXPECT_RET_FAILED(cbuff.impl, CBufferInternal<false>::Create(dev.Get().dx11, data.DataRef.get() ? data.DataRef.get() : data.DataStatic, data.Bytes));

		if (data.ResourceID != INVALID_EID)
			Settings::Data.get_or_emplace<Data::ResourceLocationAtom>(data.ResourceID) = Data::ResourceLocation::GPU;
		return cbuff;
	}

	Expected<CBuffer> CBuffer::Create(GFX::Device& dev, GFX::DiskManager& disk, const GFX::Resource::CBufferFileData& data, GFX::GFile& file) noexcept
	{
		DX::ComPtr<IBuffer> buffer;
		ZE_EXPECT_RET_FAILED(buffer, CreateCBuffer(D3D11_USAGE_DEFAULT, 0, dev.Get().dx11, nullptr, data.UncompressedSize));

		CBuffer cbuff = {};
		cbuff.impl.SetBuffer(std::move(buffer));
		disk.Get().dx11.AddFileBufferRequest(data.ResourceID, cbuff.GetBuffer(), file, data.BufferDataOffset, data.SourceBytes, data.Compression, data.UncompressedSize);
		return cbuff;
	}
}