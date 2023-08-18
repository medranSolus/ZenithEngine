#include "RHI/DX12/Resource/Mesh.h"

namespace ZE::RHI::DX12::Resource
{
	Mesh::Mesh(GFX::Device& dev, const GFX::MeshData& data)
	{
		ZE_DX_ENABLE_ID(dev.Get().dx12);

		vertexView.SizeInBytes = data.VertexCount * data.VertexSize;
		vertexView.StrideInBytes = data.VertexSize;

		if (data.Indices)
		{
			ZE_ASSERT(data.IndexSize == sizeof(U16) || data.IndexSize == sizeof(U32),
				"Only 16 and 32 bit indices are supported for DirectX 12!");

			is16bitIndices = data.IndexSize == sizeof(U16);
			indexView.SizeInBytes = data.IndexCount * data.IndexSize + GetIndexBytesOffset();
			indexView.Format = is16bitIndices ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
		}
		else
		{
			is16bitIndices = false;
			indexView.SizeInBytes = vertexView.SizeInBytes;
			indexView.Format = DXGI_FORMAT_UNKNOWN;
		}

		const D3D12_RESOURCE_DESC1 desc = dev.Get().dx12.GetBufferDesc(indexView.SizeInBytes);
		info = dev.Get().dx12.CreateBuffer(desc, false);
		indexView.BufferLocation = vertexView.BufferLocation = info.Resource->GetGPUVirtualAddress();
		ZE_DX_SET_ID(info.Resource, "Mesh geometry buffer");

		if (data.Indices)
		{
			// Pack mesh data into single buffer: vertex + index data
			std::unique_ptr<U8[]> dataBuffer = std::make_unique<U8[]>(indexView.SizeInBytes);
			memcpy(dataBuffer.get(), data.Vertices, vertexView.SizeInBytes);
			memcpy(dataBuffer.get() + GetIndexBytesOffset(), data.Indices, data.IndexCount * data.IndexSize);
			dev.Get().dx12.UploadBuffer(info.Resource.Get(), desc, dataBuffer.get(),
				indexView.SizeInBytes, D3D12_RESOURCE_STATE_INDEX_BUFFER | D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		}
		else
		{
			// When mesh without indices is present, skip second buffer entirely
			dev.Get().dx12.UploadBuffer(info.Resource.Get(), desc, data.Vertices,
				vertexView.SizeInBytes, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		}
	}

	void Mesh::Draw(GFX::Device& dev, GFX::CommandList& cl) const noexcept(!_ZE_DEBUG_GFX_API)
	{
		ZE_DX_ENABLE_INFO(dev.Get().dx12);

		IGraphicsCommandList* list = cl.Get().dx12.GetList();
		list->IASetVertexBuffers(0, 1, &vertexView);
		if (IsIndexBufferPresent())
		{
			list->IASetIndexBuffer(&indexView);
			ZE_DX_THROW_FAILED_INFO(list->DrawIndexedInstanced(GetIndexCount(), 1, GetIndexBytesOffset() / GetIndexSize(), 0, 0));
		}
		else
		{
			ZE_DX_THROW_FAILED_INFO(list->DrawInstanced(GetVertexCount(), 1, 0, 0));
		}
	}

	GFX::MeshData Mesh::GetData(GFX::Device& dev, GFX::CommandList& cl) const
	{
		return { nullptr, nullptr, 0, 0, 0, 0 };
	}
}