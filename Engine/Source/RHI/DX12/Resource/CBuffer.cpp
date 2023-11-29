#include "RHI/DX12/Resource/CBuffer.h"
#include "Data/ResourceLocation.h"

namespace ZE::RHI::DX12::Resource
{
	CBuffer::CBuffer(GFX::Device& dev, IO::DiskManager& disk, const GFX::Resource::CBufferData& data)
	{
		Device& device = dev.Get().dx12;
		ZE_DX_ENABLE_ID(device);

		const D3D12_RESOURCE_DESC1 desc = device.GetBufferDesc(data.Bytes);
		resInfo = device.CreateBuffer(desc, false);
		ZE_DX_SET_ID(resInfo.Resource, "CBuffer");
		address = resInfo.Resource->GetGPUVirtualAddress();

		Update(dev, disk, data);
	}

	CBuffer::CBuffer(GFX::Device& dev, IO::DiskManager& disk, const GFX::Resource::CBufferFileData& data, IO::File& file)
	{
		Device& device = dev.Get().dx12;
		ZE_DX_ENABLE_ID(device);

		const D3D12_RESOURCE_DESC1 desc = device.GetBufferDesc(data.UncompressedSize);
		resInfo = device.CreateBuffer(desc, false);
		ZE_DX_SET_ID(resInfo.Resource, "CBuffer from file");
		address = resInfo.Resource->GetGPUVirtualAddress();

		disk.Get().dx12.AddFileBufferRequest(data.ResourceID, file, resInfo.Resource.Get(), data.BufferDataOffset, data.SourceBytes, data.Compression, data.UncompressedSize);
	}

	void CBuffer::Update(GFX::Device& dev, IO::DiskManager& disk, const GFX::Resource::CBufferData& data) const
	{
		ZE_DX_ENABLE_ID(dev.Get().dx12);

		if (dev.Get().dx12.IsGpuUploadHeap())
		{
			// Only memcpy will suffice
			D3D12_RANGE range = {};
			void* uploadBuffer = nullptr;
			ZE_DX_THROW_FAILED(resInfo.Resource->Map(0, &range, &uploadBuffer));
			std::memcpy(uploadBuffer, data.Data, data.Bytes);
			resInfo.Resource->Unmap(0, nullptr);
			// Indicate that resource is already on GPU
			if (data.ResourceID != INVALID_EID)
				Settings::Data.get<Data::ResourceLocationAtom>(data.ResourceID) = Data::ResourceLocation::GPU;
		}
		else
			disk.Get().dx12.AddMemoryBufferRequest(data.ResourceID, resInfo.Resource.Get(), data.Data, data.Bytes);
	}

	void CBuffer::Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept
	{
		const auto& schema = bindCtx.BindingSchema.Get().dx12;
		ZE_ASSERT(schema.GetCurrentType(bindCtx.Count) == Binding::Schema::BindType::CBV,
			"Bind slot is not a constant buffer! Wrong root signature or order of bindings!");

		auto* list = cl.Get().dx12.GetList();
		if (schema.IsCompute())
			list->SetComputeRootConstantBufferView(bindCtx.Count++, address);
		else
			list->SetGraphicsRootConstantBufferView(bindCtx.Count++, address);
	}

	void CBuffer::GetData(GFX::Device& dev, void* values, U32 bytes) const
	{
	}
}