#include "RHI/DX12/Resource/Mesh.h"
#include "Data/ResourceLocation.h"

namespace ZE::RHI::DX12::Resource
{
	Mesh::~Mesh()
	{
		if (info.Handle)
		{
			ZE_ASSERT(srcDev, "No source Device for cleanup!");
			srcDev->FreeBuffer(info);
		}
	}

	Expected<Mesh> Mesh::Create(GFX::Device& dev, GFX::DiskManager& disk, const GFX::Resource::MeshData& data) noexcept
	{
		Device& device = dev.Get().dx12;

		Mesh mesh = {};
		mesh.srcDev = &device;
		mesh.vertexView.SizeInBytes = data.VertexCount * data.VertexSize;
		mesh.vertexView.StrideInBytes = data.VertexSize;

		if (data.IndexCount)
		{
			ZE_ASSERT(data.IndexSize == sizeof(U16) || data.IndexSize == sizeof(U32),
				"Only 16 and 32 bit indices are supported for DirectX 12!");

			mesh.is16bitIndices = data.IndexSize == sizeof(U16);
			mesh.indexView.SizeInBytes = data.IndexCount * data.IndexSize;
			mesh.indexView.Format = mesh.is16bitIndices ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
		}
		else
		{
			mesh.is16bitIndices = false;
			mesh.indexView.SizeInBytes = 0;
		}

		const U32 vertexOffset = Math::AlignUp(mesh.indexView.SizeInBytes, GFX::Resource::MeshData::VERTEX_BUFFER_ALIGNMENT);
		const D3D12_RESOURCE_DESC1 desc = device.GetBufferDesc(vertexOffset + mesh.vertexView.SizeInBytes);
		ZE_EXPECT_RET_FAILED(mesh.info, device.CreateBuffer(desc, false));
		mesh.indexView.BufferLocation = mesh.info.Resource->GetGPUVirtualAddress();
		mesh.vertexView.BufferLocation = mesh.indexView.BufferLocation + vertexOffset;
		ZE_DX_SET_ID(mesh.info.Resource, "Mesh geometry buffer");

		if (device.IsGpuUploadHeap())
		{
			// Only memcpy will suffice
			D3D12_RANGE range = {};
			void* uploadBuffer = nullptr;
			ZE_DX_RET_FAILED_EXPECT(mesh.info.Resource->Map(0, &range, &uploadBuffer));
			std::memcpy(uploadBuffer, data.PackedMesh.get(), desc.Width);
			mesh.info.Resource->Unmap(0, nullptr);
			// Indicate that resource is already on GPU
			if (data.MeshID != INVALID_EID)
				Settings::Data.get_or_emplace<Data::ResourceLocationAtom>(data.MeshID) = Data::ResourceLocation::GPU;
		}
		else
			disk.Get().dx12.AddMemoryBufferRequest(data.MeshID, mesh.info.Resource.Get(), nullptr, data.PackedMesh, Utils::SafeCast<U32>(desc.Width), true);
		return mesh;
	}

	Expected<Mesh> Mesh::Create(GFX::Device& dev, GFX::DiskManager& disk, const GFX::Resource::MeshFileData& data, GFX::GFile& file) noexcept
	{
		Device& device = dev.Get().dx12;

		Mesh mesh = {};
		mesh.srcDev = &device;
		mesh.vertexView.SizeInBytes = data.VertexCount * data.VertexSize;
		mesh.vertexView.StrideInBytes = data.VertexSize;

		mesh.indexView.SizeInBytes = data.UncompressedSize;
		mesh.indexView.Format = DX::GetDXFormat(data.IndexFormat);

		mesh.is16bitIndices = data.IndexCount && mesh.indexView.Format == DXGI_FORMAT_R16_UINT;
		ZE_ASSERT(mesh.is16bitIndices || mesh.indexView.Format == DXGI_FORMAT_R32_UINT || mesh.indexView.Format == DXGI_FORMAT_UNKNOWN,
			"Only 16 and 32 bit indices are supported for DirectX 12!");

		const D3D12_RESOURCE_DESC1 desc = device.GetBufferDesc(data.UncompressedSize);
		ZE_EXPECT_RET_FAILED(mesh.info, device.CreateBuffer(desc, false));
		mesh.indexView.BufferLocation = mesh.vertexView.BufferLocation = mesh.info.Resource->GetGPUVirtualAddress();
		ZE_DX_SET_ID(mesh.info.Resource, "Mesh geometry buffer from file");

		disk.Get().dx12.AddFileBufferRequest(data.MeshID, mesh.info.Resource.Get(), file, data.MeshDataOffset, data.SourceBytes, data.Compression, data.UncompressedSize, true);
		return mesh;
	}

	void Mesh::Draw(GFX::Device& dev, GFX::CommandList& cl) const noexcept
	{
		IGraphicsCommandList* list = cl.Get().dx12.GetList();
		list->IASetVertexBuffers(0, 1, &vertexView);
		if (IsIndexBufferPresent())
		{
			list->IASetIndexBuffer(&indexView);
			ZE_DX_CHECK_FAILED(list->DrawIndexedInstanced(GetIndexCount(), 1, 0, 0, 0), "DrawIndexedInstanced caused debug messages!");
		}
		else
		{
			ZE_DX_CHECK_FAILED(list->DrawInstanced(GetVertexCount(), 1, 0, 0), "DrawInstanced caused debug messages!");
		}
	}
}