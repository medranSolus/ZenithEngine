#pragma once
#include "Schema.h"

namespace ZE::GFX::Resource::Texture
{
	// Library bookkeeping all TexturePacks descriptions for preparing TexturePackDesc
	class Library final
	{
		std::unordered_map<std::string, Schema> schemas;

	public:
		Library() = default;
		ZE_CLASS_DELETE(Library);
		~Library() = default;

		bool Contains(const std::string& name) const noexcept { return schemas.contains(name); }
		const Schema& GetSchema(const std::string& name) const noexcept { ZE_ASSERT(Contains(name), "Texture schema not present!"); return schemas.at(name); }
		void AddTexturePack(const std::string& name, Schema&& schema) noexcept { ZE_ASSERT(!Contains(name), "Texture schema already present!"); schemas.emplace(name, std::forward<Schema>(schema)); }
	};
}