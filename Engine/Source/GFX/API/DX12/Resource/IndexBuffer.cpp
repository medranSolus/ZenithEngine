#include "GFX/API/DX12/Resource/IndexBuffer.h"
#include "GFX/API/DX/GraphicsException.h"

namespace ZE::GFX::API::DX12::Resource
{
	IndexBuffer::IndexBuffer(GFX::Device& dev, U32 count, U32* indices)
	{
		ZE_GFX_ENABLE_ID(dev.Get().dx12);

		D3D12_RESOURCE_DESC desc = dev.Get().dx12.GetBufferDesc(count * sizeof(U32));
		info = dev.Get().dx12.CreateBuffer(desc);
		ZE_GFX_SET_ID(info.Resource, "IndexBuffer");

		D3D12_HEAP_PROPERTIES tempHeap;
		tempHeap.Type = D3D12_HEAP_TYPE_UPLOAD;
		tempHeap.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		tempHeap.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		tempHeap.CreationNodeMask = 0;
		tempHeap.VisibleNodeMask = 0;

		DX::ComPtr<ID3D12Resource> tempRes;
		ZE_GFX_THROW_FAILED(dev.Get().dx12.GetDevice()->CreateCommittedResource(&tempHeap,
			D3D12_HEAP_FLAG_CREATE_NOT_ZEROED, &desc,
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&tempRes)));

		D3D12_RANGE range = { 0 };
		void* uploadBuffer = nullptr;
		ZE_GFX_THROW_FAILED(tempRes->Map(0, &range, &uploadBuffer));
		memcpy(uploadBuffer, indices, desc.Width);
		tempRes->Unmap(0, nullptr);
		dev.Get().dx12.CopyResource(info.Resource.Get(), std::move(tempRes));
	}

	void IndexBuffer::Bind(GFX::CommandList& cl) const noexcept
	{
	}

	U32* IndexBuffer::GetData(GFX::Device& dev, GFX::CommandList& cl) const
	{
		return nullptr;
	}
}