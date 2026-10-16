#include "RHI/DX11/Resource/Mesh.h"

namespace ZE::RHI::DX11::Resource
{
	Status Mesh::CreateMesh(Device& device, D3D11_USAGE usage, D3D11_SUBRESOURCE_DATA* resData, U32* size) noexcept
	{
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.Usage = usage;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.ByteWidth = Utils::SafeCast<UINT>(Math::AlignUp(indexCount * GetIndexSize(), GFX::Resource::MeshData::VERTEX_BUFFER_ALIGNMENT) + vertexCount * (vertexSizes.front() + vertexSizes.at(1)));
		bufferDesc.StructureByteStride = 0;

		if (indexCount)
		{
			// Pack mesh data into single buffer: index + vertex data
			ZE_ASSERT((options & GFX::Resource::MeshOption::IndexFormat8bit) == 0,
				"Only 16 and 32 bit indices are supported for DirectX 11!");

			bufferDesc.BindFlags |= D3D11_BIND_INDEX_BUFFER;
		}

		ZE_DX_RET_FAILED(device.GetDevice()->CreateBuffer(&bufferDesc, resData, &buffer));
		ZE_DX_SET_ID(buffer, "Mesh geometry buffer");

		if (size)
			*size = bufferDesc.ByteWidth;
		return {};
	}

	Expected<Mesh> Mesh::Create(GFX::Device& dev, GFX::DiskManager& disk, const GFX::Resource::MeshData& data) noexcept
	{
		Mesh mesh = {};
		mesh.vertexSizes.front() = data.VertexSize - data.VertexSizeSecondStream;
		mesh.vertexSizes.at(1) = data.VertexSizeSecondStream;
		mesh.vertexCount = data.VertexCount;
		mesh.indexCount = data.IndexCount;
		mesh.options = data.Options;

		const U32 indexOffset = mesh.GetIndexOffset();
		D3D11_SUBRESOURCE_DATA resData = {};
		resData.pSysMem = data.PackedMesh.get() + indexOffset;
		resData.SysMemPitch = 0;
		resData.SysMemSlicePitch = 0;
		ZE_CODE_RET_FAILED_EXPECT(mesh.CreateMesh(dev.Get().dx11, D3D11_USAGE_IMMUTABLE, &resData, nullptr));

		if (indexOffset)
		{
			GFX::Resource::CBufferData cbuffData = {};
			cbuffData.DataRef = data.PackedMesh;
			cbuffData.Bytes = indexOffset;
			ZE_EXPECT_RET_FAILED(mesh.decodeBuffer, CBuffer::Create(dev, disk, cbuffData));
		}

		return mesh;
	}

	Expected<Mesh> Mesh::Create(GFX::Device& dev, GFX::DiskManager& disk, const GFX::Resource::MeshFileData& data, GFX::GFile& file) noexcept
	{
		Mesh mesh = {};
		mesh.vertexSizes.front() = data.VertexSize - data.VertexSizeSecondStream;
		mesh.vertexSizes.at(1) = data.VertexSizeSecondStream;
		mesh.vertexCount = data.VertexCount;
		mesh.indexCount = data.IndexCount;
		mesh.options = data.Options;

		U32 size = 0;
		ZE_CODE_RET_FAILED_EXPECT(mesh.CreateMesh(dev.Get().dx11, D3D11_USAGE_DEFAULT, nullptr, &size));

		const U32 indexOffset = mesh.GetIndexOffset();
		if (indexOffset)
		{
			GFX::Resource::CBufferData cbuffData = {};
			cbuffData.Bytes = indexOffset;
			ZE_EXPECT_RET_FAILED(mesh.decodeBuffer, CBuffer::Create(dev, disk, cbuffData));
		}

		disk.Get().dx11.AddFileBufferRequest(mesh.buffer, file, data.MeshDataOffset, data.SourceBytes, data.Compression, size, mesh.decodeBuffer.GetResource(), indexOffset);
		return mesh;
	}

	void Mesh::BindDecodeBuffer(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept
	{
		if (decodeBuffer.GetBuffer())
			decodeBuffer.Bind(cl, bindCtx);
	}

	void Mesh::Draw(GFX::Device& dev, GFX::CommandList& cl) const noexcept
	{
		IDeviceContext* ctx = cl.Get().dx11.GetContext();

		const U32 offset = Math::AlignUp(indexCount * GetIndexSize(), GFX::Resource::MeshData::VERTEX_BUFFER_ALIGNMENT);
		std::array<U32, 2> offsets = { offset, offset + vertexSizes.front() * vertexCount };
		std::array<IBuffer*, 2> buffers = { buffer.Get(), buffer.Get() };
		ctx->IASetVertexBuffers(0, 1 + IsSecondStreamPresent(), buffers.data(), vertexSizes.data(), offsets.data());

		if (IsIndexBufferPresent())
		{
			ctx->IASetIndexBuffer(buffer.Get(), Is16BitIndices() ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
			ZE_DX_CHECK_FAILED(ctx->DrawIndexed(indexCount, 0, 0), "DrawIndexed caused debug messages!");
		}
		else
		{
			ZE_DX_CHECK_FAILED(ctx->Draw(vertexCount, 0), "Draw caused debug messages!");
		}
	}
}