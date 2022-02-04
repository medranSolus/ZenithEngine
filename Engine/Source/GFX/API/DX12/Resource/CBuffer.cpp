#include "GFX/API/DX12/Resource/CBuffer.h"

namespace ZE::GFX::API::DX12::Resource
{
	CBuffer::CBuffer(GFX::Device& dev, const void* values, U32 bytes, bool dynamic)
	{
		ZE_ASSERT(bytes != 0, "Empty buffer!");
		auto& device = dev.Get().dx12;
		ZE_GFX_ENABLE_ID(device);

		D3D12_RESOURCE_DESC desc = dev.Get().dx12.GetBufferDesc(bytes);
		resInfo = device.CreateBuffer(desc, dynamic);
		ZE_GFX_SET_ID(resInfo.Resource, "CBuffer");
		address = resInfo.Resource->GetGPUVirtualAddress();

		if (dynamic)
		{
			D3D12_RANGE range = { 0 };
			ZE_GFX_THROW_FAILED(resInfo.Resource->Map(0, &range, &buffer));
		}
		if (values)
		{
			if (dynamic)
				memcpy(buffer, values, bytes);
			else
			{
				dev.Get().dx12.UploadBuffer(resInfo.Resource.Get(), desc, values,
					bytes, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
			}
		}
	}

	void CBuffer::Update(GFX::Device& dev, const void* values, U32 bytes) const
	{
		if (buffer)
			memcpy(buffer, values, bytes);
		else
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
		if (buffer)
			resInfo.Resource->Unmap(0, nullptr);
		dev.Get().dx12.FreeBuffer(resInfo);
	}
}