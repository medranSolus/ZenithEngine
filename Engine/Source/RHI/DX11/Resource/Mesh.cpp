#include "RHI/DX11/Resource/Mesh.h"
#include "Data/ResourceLocation.h"

namespace ZE::RHI::DX11::Resource
{
	Expected<Mesh> Mesh::Create(GFX::Device& dev, GFX::DiskManager& disk, const GFX::Resource::MeshData& data) noexcept
	{
		Mesh mesh = {};
		mesh.vertexSize = data.VertexSize;
		mesh.vertexCount = data.VertexCount;
		mesh.indexCount = data.IndexCount;

		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.ByteWidth = Utils::SafeCast<UINT>(Math::AlignUp(data.IndexCount * data.IndexSize, GFX::Resource::MeshData::VERTEX_BUFFER_ALIGNMENT) + data.VertexCount * data.VertexSize);
		bufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA resData = {};
		resData.pSysMem = data.PackedMesh.get();
		resData.SysMemPitch = 0;
		resData.SysMemSlicePitch = 0;

		if (data.IndexCount)
		{
			// Pack mesh data into single buffer: index + vertex data
			ZE_ASSERT(data.IndexSize == sizeof(U16) || data.IndexSize == sizeof(U32),
				"Only 16 and 32 bit indices are supported for DirectX 11!");

			mesh.is16bitIndices = data.IndexSize == sizeof(U16);
			bufferDesc.BindFlags |= D3D11_BIND_INDEX_BUFFER;
		}
		else
			mesh.is16bitIndices = false;

		ZE_DX_RET_FAILED_EXPECT(dev.Get().dx11.GetDevice()->CreateBuffer(&bufferDesc, &resData, &mesh.buffer));
		ZE_DX_SET_ID(mesh.buffer, "Mesh geometry buffer");
		if (data.MeshID != INVALID_EID)
			Settings::Data.get_or_emplace<Data::ResourceLocationAtom>(data.MeshID) = Data::ResourceLocation::GPU;

		return mesh;
	}

	Expected<Mesh> Mesh::Create(GFX::Device& dev, GFX::DiskManager& disk, const GFX::Resource::MeshFileData& data, GFX::GFile& file) noexcept
	{
		Mesh mesh = {};
		mesh.vertexSize = data.VertexSize;
		mesh.vertexCount = data.VertexCount;
		mesh.indexCount = data.IndexCount;

		return mesh;
	}

	void Mesh::Draw(GFX::Device& dev, GFX::CommandList& cl) const noexcept
	{
		IDeviceContext* ctx = cl.Get().dx11.GetContext();

		const U32 offset = Math::AlignUp(indexCount * GetIndexSize(), GFX::Resource::MeshData::VERTEX_BUFFER_ALIGNMENT);
		ctx->IASetVertexBuffers(0, 1, buffer.GetAddressOf(), &vertexSize, &offset);

		if (IsIndexBufferPresent())
		{
			ctx->IASetIndexBuffer(buffer.Get(), is16bitIndices ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
			ZE_DX_CHECK_FAILED(ctx->DrawIndexed(indexCount, 0, 0), "DrawIndexed caused debug messages!");
		}
		else
		{
			ZE_DX_CHECK_FAILED(ctx->Draw(vertexCount, 0), "Draw caused debug messages!");
		}
	}
}