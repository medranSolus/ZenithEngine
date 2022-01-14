#pragma once
#include "TextureSchema.h"

namespace ZE::GFX::Resource
{
	// Library bookkeeping all TexturePacks descriptions for preparing TexturePackDesc
	class TextureLibrary final
	{
		std::unordered_map<std::string, TextureSchema> schemas;

	public:
		TextureLibrary() = default;
		ZE_CLASS_DELETE(TextureLibrary);
		~TextureLibrary() = default;

		bool Contains(const std::string& name) const noexcept { return schemas.contains(name); }
		const TextureSchema& GetSchema(const std::string& name) const noexcept { ZE_ASSERT(Contains(name), "Texture schema not present!"); return schemas.at(name); }
		void AddTexturePack(const std::string& name, TextureSchema&& schema) noexcept { ZE_ASSERT(!Contains(name), "Texture schema already present!"); schemas.emplace(name, std::forward<TextureSchema>(schema)); }
	};
}