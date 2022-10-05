#include "GFX/API/VK/Resource/IndexBuffer.h"

namespace ZE::GFX::API::VK::Resource
{
	IndexBuffer::IndexBuffer(GFX::Device& dev, const IndexData& data)
	{
	}

	void IndexBuffer::Bind(GFX::CommandList& cl) const noexcept
	{
	}

	IndexData IndexBuffer::GetData(GFX::Device& dev, GFX::CommandList& cl) const
	{
		return {};
	}
}