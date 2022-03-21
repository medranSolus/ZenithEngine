#include "GFX/Resource/Texture/Schema.h"

namespace ZE::GFX::Resource::Texture
{
	void Schema::AddTexture(const std::string& name, Type type, Usages usage) noexcept
	{
		ZE_ASSERT(!Location.contains(name) && !Info.contains(name), "Texture already present!");
		Location.emplace(name, static_cast<U32>(Location.size()));
		Info.emplace(name, std::make_pair(type, usage));
	}
}