#include "GFX/API/DX12/Resource/VertexBuffer.h"

namespace ZE::GFX::API::DX12::Resource
{
	VertexBuffer::VertexBuffer(GFX::Device& dev, const VertexData& data)
	{
		ZE_DX_ENABLE_ID(dev.Get().dx12);

		view.SizeInBytes = data.Count * data.VertexSize;
		view.StrideInBytes = data.VertexSize;
		const D3D12_RESOURCE_DESC1 desc = dev.Get().dx12.GetBufferDesc(view.SizeInBytes);
		info = dev.Get().dx12.CreateBuffer(desc, false);
		view.BufferLocation = info.Resource->GetGPUVirtualAddress();
		ZE_DX_SET_ID(info.Resource, "VertexBuffer");

		dev.Get().dx12.UploadBuffer(info.Resource.Get(), desc, data.Vertices,
			view.SizeInBytes, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}

	VertexData VertexBuffer::GetData(GFX::Device& dev, GFX::CommandList& cl) const
	{
		return {};
	}
}