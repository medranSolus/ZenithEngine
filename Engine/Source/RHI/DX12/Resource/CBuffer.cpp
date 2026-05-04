#include "RHI/DX12/Resource/CBuffer.h"
#include "Data/ResourceLocation.h"

namespace ZE::RHI::DX12::Resource
{
	CBuffer::~CBuffer()
	{
		if (resInfo.Handle)
		{
			ZE_ASSERT(srcDev, "No source Device for cleanup!");
			srcDev->FreeBuffer(resInfo);
		}
	}

	Expected<CBuffer> CBuffer::Create(GFX::Device& dev, GFX::DiskManager& disk, const GFX::Resource::CBufferData& data) noexcept
	{
		Device& device = dev.Get().dx12;

		CBuffer buffer = {};
		const D3D12_RESOURCE_DESC1 desc = device.GetBufferDesc(data.Bytes);
		ZE_EXPECT_RET_FAILED(buffer.resInfo, device.CreateBuffer(desc, false));
		ZE_DX_SET_ID(buffer.resInfo.Resource, "CBuffer");
		buffer.address = buffer.resInfo.Resource->GetGPUVirtualAddress();
		buffer.srcDev = &device;

		ZE_CODE_RET_FAILED_EXPECT(buffer.Update(dev, disk, data));
		return buffer;
	}

	Expected<CBuffer> CBuffer::Create(GFX::Device& dev, GFX::DiskManager& disk, const GFX::Resource::CBufferFileData& data, GFX::GFile& file) noexcept
	{
		Device& device = dev.Get().dx12;

		CBuffer buffer = {};
		const D3D12_RESOURCE_DESC1 desc = device.GetBufferDesc(data.UncompressedSize);
		ZE_EXPECT_RET_FAILED(buffer.resInfo, device.CreateBuffer(desc, false));
		ZE_DX_SET_ID(buffer.resInfo.Resource, "CBuffer from file");
		buffer.address = buffer.resInfo.Resource->GetGPUVirtualAddress();
		buffer.srcDev = &device;

		disk.Get().dx12.AddFileBufferRequest(data.ResourceID, buffer.resInfo.Resource.Get(), file, data.BufferDataOffset, data.SourceBytes, data.Compression, data.UncompressedSize, false);
		return buffer;
	}

	void CBuffer::Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept
	{
		const auto& schema = bindCtx.BindingSchema.Get().dx12;
		ZE_ASSERT(schema.GetCurrentType(bindCtx.Count) == Binding::Schema::BindType::CBV,
			"Bind slot is not a constant buffer! Wrong root signature or order of bindings!");

		auto* list = cl.Get().dx12.GetList();
		if (schema.IsCompute())
		{
			ZE_DX_CHECK_FAILED(list->SetComputeRootConstantBufferView(bindCtx.Count++, address), "Setting compute CBV resulted in debug layer messages!");
		}
		else
		{
			ZE_DX_CHECK_FAILED(list->SetGraphicsRootConstantBufferView(bindCtx.Count++, address), "Setting GFX CBV resulted in debug layer messages!");
		}
	}

	Status CBuffer::Update(GFX::Device& dev, GFX::DiskManager& disk, const GFX::Resource::CBufferData& data) const noexcept
	{
		if (dev.Get().dx12.IsGpuUploadHeap())
		{
			// Only memcpy will suffice
			D3D12_RANGE range = {};
			void* uploadBuffer = nullptr;
			ZE_DX_RET_FAILED(resInfo.Resource->Map(0, &range, &uploadBuffer));
			std::memcpy(uploadBuffer, data.DataRef.get() ? data.DataRef.get() : data.DataStatic, data.Bytes);
			resInfo.Resource->Unmap(0, nullptr);
			// Indicate that resource is already on GPU
			if (data.ResourceID != INVALID_EID)
				Settings::Data.get_or_emplace<Data::ResourceLocationAtom>(data.ResourceID) = Data::ResourceLocation::GPU;
		}
		else
			disk.Get().dx12.AddMemoryBufferRequest(data.ResourceID, resInfo.Resource.Get(), data.DataStatic, data.DataRef, data.Bytes, false);
		return {};
	}

}