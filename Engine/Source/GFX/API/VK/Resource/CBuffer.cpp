#include "GFX/API/VK/Resource/CBuffer.h"

namespace ZE::GFX::API::VK::Resource
{
	CBuffer::CBuffer(GFX::Device& dev, const void* values, U32 bytes, bool dynamic)
	{
	}

	void CBuffer::Update(GFX::Device& dev, const void* values, U32 bytes) const
	{
	}

	void CBuffer::Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept
	{
	}

	void CBuffer::Free(GFX::Device& dev) noexcept
	{
	}

	void CBuffer::GetData(GFX::Device& dev, void* values, U32 bytes) const
	{
	}
}