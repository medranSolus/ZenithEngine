#include "VertexBuffer.h"
#include "GfxExceptionMacros.h"

namespace GFX::Resource
{
	VertexBuffer::VertexBuffer(Graphics& gfx, const std::string& tag, const Data::VertexBufferData& buffer)
		: stride(static_cast<UINT>(buffer.GetLayout()->Size())), name(tag)
	{
		GFX_ENABLE_ALL(gfx);

		D3D11_BUFFER_DESC bufferDesc = { 0 };
		bufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
		bufferDesc.CPUAccessFlags = 0U;
		bufferDesc.MiscFlags = 0U;
		bufferDesc.ByteWidth = static_cast<UINT>(buffer.Bytes());
		bufferDesc.StructureByteStride = stride;
		D3D11_SUBRESOURCE_DATA resData = { 0 };
		resData.pSysMem = buffer.GetData();
		GFX_THROW_FAILED(GetDevice(gfx)->CreateBuffer(&bufferDesc, &resData, &vertexBuffer));
	}
}