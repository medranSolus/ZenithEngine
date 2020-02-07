#include "IndexBuffer.h"
#include "GfxExceptionMacros.h"

namespace GFX::Resource
{
	IndexBuffer::IndexBuffer(Graphics& gfx, const std::string& tag, const std::vector<unsigned int>& indices)
		: count(static_cast<unsigned int>(indices.size())), name(tag)
	{
		GFX_ENABLE_ALL(gfx);
		D3D11_BUFFER_DESC bufferDesc = { 0 };
		bufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;
		bufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
		bufferDesc.CPUAccessFlags = 0u;
		bufferDesc.MiscFlags = 0U;
		bufferDesc.ByteWidth = static_cast<UINT>(count * sizeof(unsigned int));
		bufferDesc.StructureByteStride = sizeof(unsigned int);
		D3D11_SUBRESOURCE_DATA resData = { 0 };
		resData.pSysMem = indices.data();
		GFX_THROW_FAILED(GetDevice(gfx)->CreateBuffer(&bufferDesc, &resData, &indexBuffer));
	}
}