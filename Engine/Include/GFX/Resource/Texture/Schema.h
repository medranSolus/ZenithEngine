#pragma once
#include "Type.h"
#include "Data/Library.h"

namespace ZE::GFX::Resource::Texture
{
	typedef U8 Usages;
	// How texture will be accessed in pipeline
	enum Usage : Usages
	{
		Invalid = 0,
		PixelShader = 1,
		NonPixelShader = 2,
		All = PixelShader | NonPixelShader
	};

	// Schema of underlaying Texture::Pack structure allowing for texture lookup by names
	struct Schema
	{
		std::unordered_map<std::string, U32> Location;
		std::unordered_map<std::string, std::pair<Type, Usages>> Info;

		void AddTexture(const std::string& name, Type type, Usages usage) noexcept;
	};

	// Library bookkeeping all TexturePacks descriptions for preparing TexturePackDesc
	typedef Data::Library<std::string, Schema> Library;
}