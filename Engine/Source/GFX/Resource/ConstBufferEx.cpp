#include "GFX/Resource/ConstBufferEx.h"
#include "Exception/GfxExceptionMacros.h"
#include "GFX/Graphics.h"

namespace ZE::GFX::Resource
{
	ConstBufferEx::ConstBufferEx(Graphics& gfx, const std::string& tag,
		const Data::CBuffer::DCBLayoutElement& root, U32 slot, const Data::CBuffer::DynamicCBuffer* buffer)
		: slot(slot), name(tag), rootLayout(root)
	{
		ZE_GFX_ENABLE_ALL(gfx);
		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0;
		bufferDesc.ByteWidth = static_cast<UINT>(rootLayout.GetByteSize());
		bufferDesc.StructureByteStride = 0;
		if (buffer)
		{
			D3D11_SUBRESOURCE_DATA resData = {};
			resData.pSysMem = buffer->GetData();
			ZE_GFX_THROW_FAILED(GetDevice(gfx)->CreateBuffer(&bufferDesc, &resData, &constantBuffer));
		}
		else
		{
			ZE_GFX_THROW_FAILED(GetDevice(gfx)->CreateBuffer(&bufferDesc, nullptr, &constantBuffer));
		}
	}

	void ConstBufferEx::Update(Graphics& gfx, const Data::CBuffer::DynamicCBuffer& buffer) const
	{
		assert(&buffer.GetRootElement() == &rootLayout);
		ZE_GFX_ENABLE_ALL(gfx);
		D3D11_MAPPED_SUBRESOURCE subres;
		ZE_GFX_THROW_FAILED(GetContext(gfx)->Map(constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subres));
		memcpy(subres.pData, buffer.GetData(), buffer.GetByteSize());
		GetContext(gfx)->Unmap(constantBuffer.Get(), 0);
	}
}