#include "GFX/Resource/TextureSchema.h"

namespace ZE::GFX::Resource
{
	void TextureSchema::AddTexture(const std::string& name, TextureType type, TextureUsage usage) noexcept
	{
		ZE_ASSERT(!Location.contains(name) && !Info.contains(name), "Texture already present!");
		Location.emplace(name, static_cast<U32>(Location.size()));
		Info.emplace(name, std::make_pair(type, usage));
	}
}