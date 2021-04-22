#include "GFX/Resource/VertexBuffer.h"
#include "GFX/Resource/GfxDebugName.h"
#include "GFX/Graphics.h"

namespace GFX::Resource
{
	VertexBuffer::VertexBuffer(Graphics& gfx, const std::string& tag, const Data::VertexBufferData& buffer)
		: stride(static_cast<U32>(buffer.GetLayout()->Size())), name(tag), boundingBox(buffer.GetBox())
	{
		GFX_ENABLE_ALL(gfx);

		D3D11_BUFFER_DESC bufferDesc = { 0 };
		bufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.ByteWidth = static_cast<UINT>(buffer.Bytes());
		bufferDesc.StructureByteStride = stride;
		D3D11_SUBRESOURCE_DATA resData = { 0 };
		resData.pSysMem = buffer.GetData();
		GFX_THROW_FAILED(GetDevice(gfx)->CreateBuffer(&bufferDesc, &resData, &vertexBuffer));
		GFX_SET_RID(vertexBuffer.Get());
	}

	GfxResPtr<VertexBuffer> VertexBuffer::Get(Graphics& gfx, const std::string& tag, const Data::VertexBufferData& buffer)
	{
		return Codex::Resolve<VertexBuffer>(gfx, tag, buffer);
	}

	void VertexBuffer::Bind(Graphics& gfx) const
	{
		const UINT offset = 0;
		GetContext(gfx)->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
	}
}