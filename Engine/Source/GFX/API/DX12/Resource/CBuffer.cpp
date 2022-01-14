#include "GFX/API/DX12/Resource/CBuffer.h"

namespace ZE::GFX::API::DX12::Resource
{
	CBuffer::CBuffer(GFX::Device& dev, const U8* values, U32 bytes, bool dynamic)
	{
		ZE_GFX_ENABLE_ID(dev.Get().dx12);

		//ZE_GFX_SET_ID(buffer, "CBuffer");
	}

	void CBuffer::Update(GFX::CommandList& cl, const U8* values, U32 bytes) const
	{
	}

	void CBuffer::UpdateDynamic(GFX::Device& dev, GFX::CommandList& cl, const U8* values, U32 bytes) const
	{
		ZE_GFX_ENABLE(dev.Get().dx12);
	}

	void CBuffer::BindVS(GFX::CommandList& cl, ShaderSlot slot) const noexcept
	{
		assert(slot < D3D12_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT);
	}

	void CBuffer::BindDS(GFX::CommandList& cl, ShaderSlot slot) const noexcept
	{
		assert(slot < D3D12_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT);
	}

	void CBuffer::BindHS(GFX::CommandList& cl, ShaderSlot slot) const noexcept
	{
		assert(slot < D3D12_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT);
	}

	void CBuffer::BindGS(GFX::CommandList& cl, ShaderSlot slot) const noexcept
	{
		assert(slot < D3D12_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT);
	}

	void CBuffer::BindPS(GFX::CommandList& cl, ShaderSlot slot) const noexcept
	{
		assert(slot < D3D12_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT);
	}

	void CBuffer::BindCS(GFX::CommandList& cl, ShaderSlot slot) const noexcept
	{
		assert(slot < D3D12_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT);
	}
}