#pragma once
#include "Data/Library.h"
#include "Type.h"

namespace ZE::GFX::Resource::Texture
{
	// Schema of underlaying Texture::Pack structure allowing for texture lookup by names
	struct Schema
	{
		std::unordered_map<std::string, U32> Location;
		std::vector<Type> TypeInfo;

		void AddTexture(const std::string& name, Type type) noexcept;
	};

	// Library bookkeeping all TexturePacks descriptions for preparing TexturePackDesc
	typedef Data::Library<std::string, Schema> Library;
}