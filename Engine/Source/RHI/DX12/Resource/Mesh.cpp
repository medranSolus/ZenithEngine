#include "RHI/DX12/Resource/Mesh.h"
#include "RHI/DX12/GarbageCollector.h"

namespace ZE::RHI::DX12::Resource
{
	Expected<U32> Mesh::CreateMesh(Device& device, U32 vertexCount, U16 vertexSize, U32 indexCount) noexcept
	{
		vertexView.SizeInBytes = vertexCount * vertexSize;
		vertexView.StrideInBytes = vertexSize;

		if (indexCount)
		{
			ZE_ASSERT((options & GFX::Resource::MeshOption::IndexFormat8bit) == 0,
				"Only 16 and 32 bit indices are supported for DirectX 12!");

			indexView.SizeInBytes = indexCount * GetIndexSize();
			indexView.Format = Is16BitIndices() ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
		}
		else
			indexView.SizeInBytes = 0;

		const U32 indexOffset = GetIndexOffset();
		const U32 vertexOffset = Math::AlignUp(indexView.SizeInBytes + indexOffset, GFX::Resource::MeshData::VERTEX_BUFFER_ALIGNMENT);
		const D3D12_RESOURCE_DESC1 desc = device.GetBufferDesc(vertexOffset + vertexView.SizeInBytes);
		ZE_EXPECT_RET_FAILED(info, device.CreateBuffer(desc, false));
		GarbageCollector::Get().MarkActive(device, info.Handle);

		indexView.BufferLocation = info.Resource->GetGPUVirtualAddress() + indexOffset;
		vertexView.BufferLocation = indexView.BufferLocation + vertexOffset - indexOffset;
		ZE_DX_SET_ID(info.Resource, "Mesh geometry buffer");

		return Utils::SafeCast<U32>(desc.Width);
	}

	Mesh::~Mesh()
	{
		if (info.Handle)
			GarbageCollector::Get().RegisterBuffer(GarbageCollector::Get().MarkInactive(info.Handle), std::move(info));
	}

	Expected<Mesh> Mesh::Create(GFX::Device& dev, GFX::DiskManager& disk, const GFX::Resource::MeshData& data) noexcept
	{
		Device& device = dev.Get().dx12;

		Mesh mesh = {};
		mesh.options = data.Options;
		U32 bufferSize = 0;
		ZE_EXPECT_RET_FAILED(bufferSize, mesh.CreateMesh(device, data.VertexCount, data.VertexSize, data.IndexCount));

		if (device.IsGpuUploadHeap())
		{
			// Only memcpy will suffice
			D3D12_RANGE range = {};
			void* uploadBuffer = nullptr;
			ZE_DX_RET_FAILED_EXPECT(mesh.info.Resource->Map(0, &range, &uploadBuffer));
			std::memcpy(uploadBuffer, data.PackedMesh.get(), bufferSize);
			mesh.info.Resource->Unmap(0, nullptr);
		}
		else
			disk.Get().dx12.AddMemoryBufferRequest(mesh.info.Resource.Get(), nullptr, data.PackedMesh, bufferSize, true);
		return mesh;
	}

	Expected<Mesh> Mesh::Create(GFX::Device& dev, GFX::DiskManager& disk, const GFX::Resource::MeshFileData& data, GFX::GFile& file) noexcept
	{
		Device& device = dev.Get().dx12;

		Mesh mesh = {};
		mesh.options = data.Options;
		U32 bufferSize = 0;
		ZE_EXPECT_RET_FAILED(bufferSize, mesh.CreateMesh(device, data.VertexCount, data.VertexSize, data.IndexCount));

		disk.Get().dx12.AddFileBufferRequest(mesh.info.Resource.Get(), file, data.MeshDataOffset, data.SourceBytes, data.Compression, bufferSize, true);
		return mesh;
	}

	void Mesh::BindDecodeBuffer(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept
	{
		U32 indexOffset = GetIndexOffset();
		if (indexOffset)
		{
			const auto& schema = bindCtx.BindingSchema.Get().dx12;
			ZE_ASSERT(schema.GetCurrentType(bindCtx.Count) == Binding::Schema::BindType::CBV,
				"Bind slot is not a constant buffer! Wrong root signature or order of bindings!");

			ZE_DX_CHECK_FAILED(cl.Get().dx12.GetList()->SetGraphicsRootConstantBufferView(bindCtx.Count++, indexView.BufferLocation - indexOffset),
				"Setting vertex decoding cbuffer resulted in debug layer messages!");
		}
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