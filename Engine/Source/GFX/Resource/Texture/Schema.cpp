#include "GFX/Resource/Texture/Schema.h"

namespace ZE::GFX::Resource::Texture
{
	void Schema::AddTexture(const std::string& name, Type type) noexcept
	{
		ZE_ASSERT(!Location.contains(name), "Texture already present!");
		const U32 location = Utils::SafeCast<U32>(TypeInfo.size());
		TypeInfo.emplace_back(type);
		Location.emplace(name, location);
	}
}