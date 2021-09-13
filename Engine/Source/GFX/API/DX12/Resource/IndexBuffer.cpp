#include "GFX/API/DX12/Resource/IndexBuffer.h"
#include "GFX/API/DX/GraphicsException.h"

namespace ZE::GFX::API::DX12::Resource
{
	IndexBuffer::IndexBuffer(GFX::Device& dev, const IndexData& data)
	{
		assert(data.Indices != nullptr && data.Count != 0 && data.Count % 3 == 0);
		ZE_GFX_ENABLE_ID(dev.Get().dx12);

		view.SizeInBytes = data.Count * sizeof(U32);
		view.Format = DXGI_FORMAT_R32_UINT;
		D3D12_RESOURCE_DESC desc = dev.Get().dx12.GetBufferDesc(view.SizeInBytes);
		info = dev.Get().dx12.CreateBuffer(desc);
		view.BufferLocation = info.Resource->GetGPUVirtualAddress();
		ZE_GFX_SET_ID(info.Resource, "IndexBuffer");

		dev.Get().dx12.CopyResource(info.Resource.Get(), desc, data.Indices, view.SizeInBytes);
	}

	IndexData IndexBuffer::GetData(GFX::Device& dev, GFX::CommandList& cl) const
	{
		return { 0, nullptr };
	}
}