#include "RHI/DX12/Resource/Mesh.h"
#include "Data/ResourceLocation.h"

namespace ZE::RHI::DX12::Resource
{
	Mesh::Mesh(GFX::Device& dev, IO::DiskManager& disk, const GFX::Resource::MeshData& data)
	{
		Device& device = dev.Get().dx12;
		ZE_DX_ENABLE_ID(device);

		vertexView.SizeInBytes = data.VertexCount * data.VertexSize;
		vertexView.StrideInBytes = data.VertexSize;

		if (data.IndexCount)
		{
			ZE_ASSERT(data.IndexSize == sizeof(U16) || data.IndexSize == sizeof(U32),
				"Only 16 and 32 bit indices are supported for DirectX 12!");

			is16bitIndices = data.IndexSize == sizeof(U16);
			indexView.SizeInBytes = data.IndexCount * data.IndexSize;
			indexView.Format = is16bitIndices ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
		}
		else
		{
			is16bitIndices = false;
			indexView.SizeInBytes = 0;
		}

		const U32 vertexOffset = Math::AlignUp(indexView.SizeInBytes, GFX::Resource::MeshData::VERTEX_BUFFER_ALIGNMENT);
		const D3D12_RESOURCE_DESC1 desc = device.GetBufferDesc(vertexOffset + vertexView.SizeInBytes);
		info = device.CreateBuffer(desc, false);
		indexView.BufferLocation = info.Resource->GetGPUVirtualAddress();
		vertexView.BufferLocation = indexView.BufferLocation + vertexOffset;
		ZE_DX_SET_ID(info.Resource, "Mesh geometry buffer");

		if (device.IsGpuUploadHeap())
		{
			// Only memcpy will suffice
			D3D12_RANGE range = {};
			void* uploadBuffer = nullptr;
			ZE_DX_THROW_FAILED(info.Resource->Map(0, &range, &uploadBuffer));
			std::memcpy(uploadBuffer, data.PackedMesh.get(), desc.Width);
			info.Resource->Unmap(0, nullptr);
			// Indicate that resource is already on GPU
			if (data.MeshID != INVALID_EID)
				Settings::Data.get_or_emplace<Data::ResourceLocationAtom>(data.MeshID) = Data::ResourceLocation::GPU;
		}
		else
			disk.Get().dx12.AddMemoryBufferRequest(data.MeshID, info.Resource.Get(), nullptr, data.PackedMesh, Utils::SafeCast<U32>(desc.Width), true);
	}

	Mesh::Mesh(GFX::Device& dev, IO::DiskManager& disk, const GFX::Resource::MeshFileData& data, IO::File& file)
	{
		ZE_DX_ENABLE_ID(dev.Get().dx12);

		vertexView.SizeInBytes = data.VertexCount * data.VertexSize;
		vertexView.StrideInBytes = data.VertexSize;

		indexView.SizeInBytes = data.UncompressedSize;
		indexView.Format = DX::GetDXFormat(data.IndexFormat);

		is16bitIndices = data.IndexCount && indexView.Format == DXGI_FORMAT_R16_UINT;
		ZE_ASSERT(is16bitIndices || indexView.Format == DXGI_FORMAT_R32_UINT || indexView.Format == DXGI_FORMAT_UNKNOWN,
			"Only 16 and 32 bit indices are supported for DirectX 12!");

		const D3D12_RESOURCE_DESC1 desc = dev.Get().dx12.GetBufferDesc(data.UncompressedSize);
		info = dev.Get().dx12.CreateBuffer(desc, false);
		indexView.BufferLocation = vertexView.BufferLocation = info.Resource->GetGPUVirtualAddress();
		ZE_DX_SET_ID(info.Resource, "Mesh geometry buffer from file");

		disk.Get().dx12.AddFileBufferRequest(data.MeshID, info.Resource.Get(), file, data.MeshDataOffset, data.SourceBytes, data.Compression, data.UncompressedSize, true);
	}

	void Mesh::Draw(GFX::Device& dev, GFX::CommandList& cl) const noexcept(!_ZE_DEBUG_GFX_API)
	{
		ZE_DX_ENABLE_INFO(dev.Get().dx12);

		IGraphicsCommandList* list = cl.Get().dx12.GetList();
		list->IASetVertexBuffers(0, 1, &vertexView);
		if (IsIndexBufferPresent())
		{
			list->IASetIndexBuffer(&indexView);
			ZE_DX_THROW_FAILED_INFO(list->DrawIndexedInstanced(GetIndexCount(), 1, 0, 0, 0));
		}
		else
		{
			ZE_DX_THROW_FAILED_INFO(list->DrawInstanced(GetVertexCount(), 1, 0, 0));
		}
	}

	GFX::Resource::MeshData Mesh::GetData(GFX::Device& dev, GFX::CommandList& cl) const
	{
		return { INVALID_EID, nullptr, 0, 0, 0, 0 };
	}
}