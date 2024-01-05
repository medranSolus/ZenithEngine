#include "GFX/Resource/Texture/Schema.h"

namespace ZE::GFX::Resource::Texture
{
	void Schema::AddTexture(const std::string& name, Type type) noexcept
	{
		ZE_ASSERT(!Location.contains(name) && !Info.contains(name), "Texture already present!");
		Location.emplace(name, Utils::SafeCast<U32>(Location.size()));
		Info.emplace(name, type);
	}
}