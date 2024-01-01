#include "GFX/Resource/Texture/PackDesc.h"

namespace ZE::GFX::Resource::Texture
{
	void PackDesc::Init(const Schema& schema) noexcept
	{
		Textures.resize(schema.Location.size());

		for (const auto& lookup : schema.Location)
		{
			auto& info = schema.Info.at(lookup.first);
			auto& tex = Textures.at(lookup.second);
			tex.Type = info.first;
			tex.Usage = info.second;
			ZE_ASSERT(tex.Usage != Usage::Invalid, "Texture usage not initialized!");
		}
	}

	void PackDesc::AddTexture(const Schema& schema, const std::string& name, std::vector<Surface>&& surfaces) noexcept
	{
		ZE_ASSERT(schema.Location.contains(name), "Specified texture not present in current schema!");

		// Check for correct Surface sizes
#if !_ZE_MODE_RELEASE
		if (surfaces.size())
		{
			ZE_ASSERT(surfaces.front().GetDepth() > 1 && surfaces.size() == 1,
				"When providing Surface with depth levels it must be the single one in the chain!");
			ZE_ASSERT(surfaces.front().GetMipCount() > 1 && surfaces.size() == 1,
				"Texture arrays with provided mip map chain should be coming from single Surface!");
			for (const Surface& surf : surfaces)
			{
				ZE_ASSERT(surf.GetWidth() == surfaces.front().GetWidth() && surf.GetHeight() == surfaces.front().GetHeight(),
					"When given more than single surface for texture, all others must match first surface dimmensions!");
				ZE_ASSERT(surf.GetFormat() == surfaces.front().GetFormat(),
					"All surfaces need to have same pixel format!");
				ZE_ASSERT(surfaces.size() == 1 || surf.GetArraySize() == 1,
					"Multiple array layers are not available when using multiple input Surfaces!");
			}
		}
#endif
		Textures.at(schema.Location.at(name)).Surfaces = std::forward<std::vector<Surface>>(surfaces);
	}

	void PackDesc::AddTexture(Type type, Usage usage, std::vector<Surface>&& surfaces) noexcept
	{
		Textures.emplace_back(type, usage, std::forward<std::vector<Surface>>(surfaces));
	}
}