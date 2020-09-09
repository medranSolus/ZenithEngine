#include "ConstBufferEx.h"
#include "GfxExceptionMacros.h"

namespace GFX::Resource
{
	ConstBufferEx::ConstBufferEx(Graphics& gfx, const std::string& tag, const Data::CBuffer::DCBLayoutElement& root,
		UINT slot, const Data::CBuffer::DynamicCBuffer* buffer)
		: slot(slot), name(tag), rootLayout(root)
	{
		GFX_ENABLE_ALL(gfx);
		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0U;
		bufferDesc.ByteWidth = static_cast<UINT>(rootLayout.GetByteSize());
		bufferDesc.StructureByteStride = 0U;
		if (buffer)
		{
			D3D11_SUBRESOURCE_DATA resData = {};
			resData.pSysMem = buffer->GetData();
			GFX_THROW_FAILED(GetDevice(gfx)->CreateBuffer(&bufferDesc, &resData, &constantBuffer));
		}
		else
		{
			GFX_THROW_FAILED(GetDevice(gfx)->CreateBuffer(&bufferDesc, nullptr, &constantBuffer));
		}
	}

	void ConstBufferEx::Update(Graphics& gfx, const Data::CBuffer::DynamicCBuffer& buffer)
	{
		assert(&buffer.GetRootElement() == &rootLayout);
		GFX_ENABLE_ALL(gfx);
		D3D11_MAPPED_SUBRESOURCE subres;
		GFX_THROW_FAILED(GetContext(gfx)->Map(constantBuffer.Get(), 0U, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0U, &subres));
		memcpy(subres.pData, buffer.GetData(), buffer.GetByteSize());
		GetContext(gfx)->Unmap(constantBuffer.Get(), 0u);
	}
}