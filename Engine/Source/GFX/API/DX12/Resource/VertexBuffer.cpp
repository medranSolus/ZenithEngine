#include "GFX/API/DX12/Resource/VertexBuffer.h"
#include "GFX/API/DX/GraphicsException.h"

namespace ZE::GFX::API::DX12::Resource
{
	VertexBuffer::VertexBuffer(GFX::Device& dev, const VertexData& data)
	{
		assert(data.Vertices != nullptr && data.BufferSize != 0 && data.VertexSize != 0);
		ZE_GFX_ENABLE_ID(dev.Get().dx12);

		view.SizeInBytes = data.BufferSize;
		view.StrideInBytes = data.VertexSize;
		D3D12_RESOURCE_DESC desc = dev.Get().dx12.GetBufferDesc(view.SizeInBytes);
		info = dev.Get().dx12.CreateBuffer(desc);
		view.BufferLocation = info.Resource->GetGPUVirtualAddress();
		ZE_GFX_SET_ID(info.Resource, "VertexBuffer");

		dev.Get().dx12.UploadResource(info.Resource.Get(), desc, data.Vertices, data.BufferSize);
	}

	VertexData VertexBuffer::GetData(GFX::Device& dev, GFX::CommandList& cl) const
	{
		return {};
	}
}