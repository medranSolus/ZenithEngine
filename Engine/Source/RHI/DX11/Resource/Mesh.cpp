#include "RHI/DX11/Resource/Mesh.h"
#include "Data/ResourceLocation.h"

namespace ZE::RHI::DX11::Resource
{
	Mesh::Mesh(GFX::Device& dev, IO::DiskManager& disk, const GFX::Resource::MeshData& data)
		: vertexSize(data.VertexSize), vertexCount(data.VertexCount)
	{
		ZE_DX_ENABLE_ID(dev.Get().dx11);

		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.ByteWidth = Utils::SafeCast<UINT>(data.VertexCount * data.VertexSize);
		bufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA resData = {};
		resData.pSysMem = data.Indices;
		resData.SysMemPitch = 0;
		resData.SysMemSlicePitch = 0;

		std::unique_ptr<U8[]> dataBuffer = nullptr;
		if (data.Indices)
		{
			// Pack mesh data into single buffer: index + vertex data
			ZE_ASSERT(data.IndexSize == sizeof(U16) || data.IndexSize == sizeof(U32),
				"Only 16 and 32 bit indices are supported for DirectX 11!");

			indexCount = data.IndexCount;
			is16bitIndices = data.IndexSize == sizeof(U16);

			const U32 vertexOffset = data.IndexCount * data.IndexSize;
			bufferDesc.BindFlags |= D3D11_BIND_INDEX_BUFFER;
			bufferDesc.ByteWidth += vertexOffset;

			dataBuffer = std::make_unique<U8[]>(bufferDesc.ByteWidth);
			memcpy(dataBuffer.get(), data.Indices, vertexOffset);
			memcpy(dataBuffer.get() + vertexOffset, data.Vertices, bufferDesc.ByteWidth - vertexOffset);

			resData.pSysMem = dataBuffer.get();
		}
		else
			resData.pSysMem = data.Vertices;

		ZE_DX_THROW_FAILED(dev.Get().dx11.GetDevice()->CreateBuffer(&bufferDesc, &resData, &buffer));
		ZE_DX_SET_ID(buffer, "Mesh geometry buffer");
		if (data.MeshID != INVALID_EID)
			Settings::Data.get<Data::ResourceLocationAtom>(data.MeshID) = Data::ResourceLocation::GPU;
	}

	Mesh::Mesh(GFX::Device& dev, IO::DiskManager& disk, const GFX::Resource::MeshFileData& data, IO::File& file)
		: vertexSize(data.VertexSize), vertexCount(data.VertexCount)
	{
	}

	void Mesh::Draw(GFX::Device& dev, GFX::CommandList& cl) const noexcept(!_ZE_DEBUG_GFX_API)
	{
		ZE_DX_ENABLE_INFO(dev.Get().dx11);

		IDeviceContext* ctx = cl.Get().dx11.GetContext();
		const U32 offset = indexCount * GetIndexSize();
		ctx->IASetVertexBuffers(0, 1, buffer.GetAddressOf(), &vertexSize, &offset);
		if (IsIndexBufferPresent())
		{
			ctx->IASetIndexBuffer(buffer.Get(), is16bitIndices ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
			ZE_DX_THROW_FAILED_INFO(ctx->DrawIndexed(indexCount, 0, 0));
		}
		else
		{
			ZE_DX_THROW_FAILED_INFO(ctx->Draw(vertexCount, 0));
		}
	}

	GFX::Resource::MeshData Mesh::GetData(GFX::Device& dev, GFX::CommandList& cl) const
	{
		return { INVALID_EID, nullptr, nullptr, 0, 0, 0, 0 };
	}
}