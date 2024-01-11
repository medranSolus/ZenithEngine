#include "GFX/Resource/Texture/PackDesc.h"

namespace ZE::GFX::Resource::Texture
{
	void PackDesc::Init(const Schema& schema) noexcept
	{
		Textures.resize(schema.Location.size());
		for (U32 i = 0; i < schema.TypeInfo.size(); ++i)
			Textures.at(i).Type = schema.TypeInfo.at(i);
	}

	void PackDesc::AddTexture(const Schema& schema, const std::string& name, std::vector<Surface>&& surfaces) noexcept
	{
		ZE_ASSERT(schema.Location.contains(name), "Specified texture not present in current schema!");

		// Check for correct Surface sizes
#if !_ZE_MODE_RELEASE
		if (surfaces.size())
		{
			ZE_ASSERT((surfaces.front().GetDepth() > 1 && surfaces.size() == 1) || surfaces.front().GetDepth() == 1,
				"When providing Surface with depth levels it must be the single one in the chain!");
			ZE_ASSERT((surfaces.front().GetMipCount() > 1 && surfaces.size() == 1) || surfaces.front().GetMipCount() == 1,
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

	void PackDesc::AddTexture(Type type, std::vector<Surface>&& surfaces) noexcept
	{
		Textures.emplace_back(type, std::forward<std::vector<Surface>>(surfaces));
	}

	void PackFileDesc::Init(const Schema& schema) noexcept
	{
		Textures.resize(schema.Location.size());
		for (U32 i = 0; i < schema.TypeInfo.size(); ++i)
			Textures.at(i).Type = schema.TypeInfo.at(i);
	}

	void PackFileDesc::AddTexture(U16 requestedlocation, const FileDesc& textureDesc) noexcept
	{
		if (requestedlocation < Textures.size())
			Textures.at(requestedlocation) = textureDesc;
		else
			Textures.emplace_back(textureDesc);
	}
}