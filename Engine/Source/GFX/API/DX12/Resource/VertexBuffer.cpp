#include "GFX/API/DX12/Resource/VertexBuffer.h"
#include "GFX/API/DX/GraphicsException.h"

namespace ZE::GFX::API::DX12::Resource
{
	VertexBuffer::VertexBuffer(GFX::Device& dev, const VertexData& data)
	{
		ZE_ASSERT(data.Vertices != nullptr && data.BufferSize != 0 && data.VertexSize != 0,
			"Empty vertex buffer!");
		ZE_GFX_ENABLE_ID(dev.Get().dx12);

		view.SizeInBytes = data.BufferSize;
		view.StrideInBytes = data.VertexSize;
		D3D12_RESOURCE_DESC desc = dev.Get().dx12.GetBufferDesc(view.SizeInBytes);
		info = dev.Get().dx12.CreateBuffer(desc, false);
		view.BufferLocation = info.Resource->GetGPUVirtualAddress();
		ZE_GFX_SET_ID(info.Resource, "VertexBuffer");

		dev.Get().dx12.UploadBuffer(info.Resource.Get(), desc, data.Vertices,
			data.BufferSize, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}

	VertexData VertexBuffer::GetData(GFX::Device& dev, GFX::CommandList& cl) const
	{
		return {};
	}
}