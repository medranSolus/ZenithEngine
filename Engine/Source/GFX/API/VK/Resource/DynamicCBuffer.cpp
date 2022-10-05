#include "GFX/API/VK/Resource/DynamicCBuffer.h"

namespace ZE::GFX::API::VK::Resource
{
	void DynamicCBuffer::AllocBlock(GFX::Device& dev)
	{
	}

	GFX::Resource::DynamicBufferAlloc DynamicCBuffer::Alloc(GFX::Device& dev, const void* values, U32 bytes)
	{
		GFX::Resource::DynamicBufferAlloc info
		{
		};
		return info;
	}

	void DynamicCBuffer::Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, const GFX::Resource::DynamicBufferAlloc& allocInfo) const noexcept
	{
	}

	void DynamicCBuffer::StartFrame(GFX::Device& dev)
	{
	}
}