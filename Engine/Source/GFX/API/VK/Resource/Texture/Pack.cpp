#include "GFX/API/VK/Resource/Texture/Pack.h"

namespace ZE::GFX::API::VK::Resource::Texture
{
	Pack::Pack(GFX::Device& dev, const GFX::Resource::Texture::PackDesc& desc)
	{
	}

	Pack::~Pack()
	{
	}

	void Pack::Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept
	{
	}

	void Pack::Free(GFX::Device& dev) noexcept
	{
	}

	std::vector<std::vector<Surface>> Pack::GetData(GFX::Device& dev) const
	{
		std::vector<std::vector<Surface>> vec;
		vec.emplace_back(std::vector<Surface>());
		vec.front().emplace_back(1U, 1U);
		return vec;
	}
}