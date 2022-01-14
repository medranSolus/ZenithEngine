#include "GFX/API/DX12/Resource/CBuffer.h"

namespace ZE::GFX::API::DX12::Resource
{
	CBuffer::CBuffer(GFX::Device& dev, const U8* values, U32 bytes, bool dynamic)
	{
		ZE_ASSERT(values != nullptr && bytes != 0, "Empty buffer!");
		auto& device = dev.Get().dx12;
		ZE_GFX_ENABLE_ID(device);

		D3D12_RESOURCE_DESC desc = dev.Get().dx12.GetBufferDesc(bytes);
		resInfo = device.CreateBuffer(desc, dynamic);
		ZE_GFX_SET_ID(resInfo.Resource, "CBuffer");
		address = resInfo.Resource->GetGPUVirtualAddress();

		dev.Get().dx12.UploadBuffer(resInfo.Resource.Get(), desc, values,
			bytes, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		if (dynamic)
		{
			D3D12_RANGE range = { 0 };
			ZE_GFX_THROW_FAILED(resInfo.Resource->Map(0, &range, &buffer));
		}
	}

	void CBuffer::Update(GFX::Device& dev, GFX::CommandList& cl, const U8* values, U32 bytes) const
	{
		if (buffer)
			memcpy(buffer, values, bytes);
		else
		{
			D3D12_RESOURCE_BARRIER barrier;
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.pResource = resInfo.Resource.Get();
			barrier.Transition.Subresource = 0;
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;

			auto* list = cl.Get().dx12.GetList();
			list->ResourceBarrier(1, &barrier);
			D3D12_RESOURCE_DESC desc = dev.Get().dx12.GetBufferDesc(bytes);
			dev.Get().dx12.UploadBuffer(resInfo.Resource.Get(), desc, values,
				bytes, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		}
	}

	void CBuffer::Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept
	{
		ZE_ASSERT(bindCtx.BindingSchema.Get().dx12.GetCurrentType(bindCtx.Count) == Binding::Schema::BindType::CBV,
			"Bind slot is not a constant buffer! Wrong root signature or order of bindings!");

		auto* list = cl.Get().dx12.GetList();
		if (bindCtx.BindingSchema.Get().dx12.IsCompute())
			list->SetComputeRootConstantBufferView(bindCtx.Count++, address);
		else
			list->SetGraphicsRootConstantBufferView(bindCtx.Count++, address);
	}
}