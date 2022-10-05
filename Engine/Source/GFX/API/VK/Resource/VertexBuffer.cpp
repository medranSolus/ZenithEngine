#include "GFX/API/VK/Resource/VertexBuffer.h"

namespace ZE::GFX::API::VK::Resource
{
	VertexBuffer::VertexBuffer(GFX::Device& dev, const VertexData& data)
	{
	}

	void VertexBuffer::Bind(GFX::CommandList& cl) const noexcept
	{
	}

	VertexData VertexBuffer::GetData(GFX::Device& dev, GFX::CommandList& cl) const
	{
		return {};
	}
}