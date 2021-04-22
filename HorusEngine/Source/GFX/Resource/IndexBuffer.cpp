#include "GFX/Resource/IndexBuffer.h"
#include "GFX/Resource/GfxDebugName.h"
#include "GFX/Graphics.h"

namespace GFX::Resource
{
	IndexBuffer::IndexBuffer(Graphics& gfx, const std::string& tag, const std::vector<U32>& indices)
		: count(static_cast<unsigned int>(indices.size())), name(tag)
	{
		GFX_ENABLE_ALL(gfx);
		D3D11_BUFFER_DESC bufferDesc = { 0 };
		bufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;
		bufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.ByteWidth = static_cast<UINT>(count * sizeof(U32));
		bufferDesc.StructureByteStride = sizeof(U32);
		D3D11_SUBRESOURCE_DATA resData = { 0 };
		resData.pSysMem = indices.data();
		GFX_THROW_FAILED(GetDevice(gfx)->CreateBuffer(&bufferDesc, &resData, &indexBuffer));
		GFX_SET_RID(indexBuffer.Get());
	}

	GfxResPtr<IndexBuffer> IndexBuffer::Get(Graphics& gfx, const std::string& tag, const std::vector<U32>& indices)
	{
		return Codex::Resolve<IndexBuffer>(gfx, tag, indices);
	}
}