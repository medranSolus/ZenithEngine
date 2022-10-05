#include "GFX/API/VK/Binding/Schema.h"

namespace ZE::GFX::API::VK::Binding
{
	Schema::Schema(GFX::Device& dev, const GFX::Binding::SchemaDesc& desc)
	{
	}

	Schema::~Schema()
	{
	}

	void Schema::SetCompute(GFX::CommandList& cl) const noexcept
	{
	}

	void Schema::SetGraphics(GFX::CommandList& cl) const noexcept
	{
	}
}