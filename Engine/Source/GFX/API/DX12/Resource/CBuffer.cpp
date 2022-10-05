#include "GFX/API/DX12/Resource/CBuffer.h"

namespace ZE::GFX::API::DX12::Resource
{
	CBuffer::CBuffer(GFX::Device& dev, const void* values, U32 bytes)
	{
		ZE_ASSERT(bytes != 0, "Empty buffer!");
		auto& device = dev.Get().dx12;
		ZE_DX_ENABLE_ID(device);

		const D3D12_RESOURCE_DESC desc = dev.Get().dx12.GetBufferDesc(bytes);
		resInfo = device.CreateBuffer(desc, false);
		ZE_DX_SET_ID(resInfo.Resource, "CBuffer");
		address = resInfo.Resource->GetGPUVirtualAddress();

		if (values)
		{
			dev.Get().dx12.UploadBuffer(resInfo.Resource.Get(), desc, values,
				bytes, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		}
	}

	void CBuffer::Update(GFX::Device& dev, const void* values, U32 bytes) const
	{
		dev.Get().dx12.UpdateBuffer(resInfo.Resource.Get(), values, bytes, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
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

	void CBuffer::Free(GFX::Device& dev) noexcept
	{
		dev.Get().dx12.FreeBuffer(resInfo);
	}

	void CBuffer::GetData(GFX::Device& dev, void* values, U32 bytes) const
	{
	}
}