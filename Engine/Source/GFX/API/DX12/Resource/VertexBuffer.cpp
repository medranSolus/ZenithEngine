#include "GFX/API/DX12/Resource/VertexBuffer.h"
#include "GFX/API/DX/GraphicsException.h"

namespace ZE::GFX::API::DX12::Resource
{
	VertexBuffer::VertexBuffer(GFX::Device& dev, const VertexData& data)
	{
		assert(data.Vertices != nullptr && data.BufferSize != 0 && data.VertexSize != 0);
		ZE_GFX_ENABLE_ID(dev.Get().dx12);

		//ZE_GFX_SET_ID(buffer, "VertexBuffer");
	}

	void VertexBuffer::Bind(GFX::CommandList& cl) const noexcept
	{
	}

	VertexData VertexBuffer::GetData(GFX::Device& dev, GFX::CommandList& cl) const
	{
		return {};
	}
}