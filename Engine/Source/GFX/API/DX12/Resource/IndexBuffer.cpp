#include "GFX/API/DX12/Resource/IndexBuffer.h"

namespace ZE::GFX::API::DX12::Resource
{
	IndexBuffer::IndexBuffer(GFX::Device& dev, const IndexData& data)
		: is16bit(data.IndexSize == sizeof(U16))
	{
		ZE_ASSERT(data.IndexSize == sizeof(U16) || data.IndexSize == sizeof(U32),
			"Only 16 and 32 bit indices are supported for DirectX 12!");
		ZE_DX_ENABLE_ID(dev.Get().dx12);

		view.SizeInBytes = data.Count * data.IndexSize;
		view.Format = is16bit ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
		const D3D12_RESOURCE_DESC1 desc = dev.Get().dx12.GetBufferDesc(view.SizeInBytes);
		info = dev.Get().dx12.CreateBuffer(desc, false);
		view.BufferLocation = info.Resource->GetGPUVirtualAddress();
		ZE_DX_SET_ID(info.Resource, "IndexBuffer");

		dev.Get().dx12.UploadBuffer(info.Resource.Get(), desc, data.Indices,
			view.SizeInBytes, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	}

	IndexData IndexBuffer::GetData(GFX::Device& dev, GFX::CommandList& cl) const
	{
		return { 0, 0, nullptr };
	}
}