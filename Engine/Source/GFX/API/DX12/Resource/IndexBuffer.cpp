#include "GFX/API/DX12/Resource/IndexBuffer.h"
#include "GFX/API/DX/GraphicsException.h"

namespace ZE::GFX::API::DX12::Resource
{
	IndexBuffer::IndexBuffer(GFX::Device& dev, U32 count, U32* indices)
	{
		ZE_GFX_ENABLE_ID(dev.Get().dx12);

		//ZE_GFX_SET_ID(buffer, "IndexBuffer");
	}

	void IndexBuffer::Bind(GFX::CommandList& cl) const noexcept
	{
	}

	U32* IndexBuffer::GetData(GFX::Device& dev, GFX::CommandList& cl) const
	{
		return nullptr;
	}
}