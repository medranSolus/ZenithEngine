#pragma once
#include "TextureType.h"

namespace ZE::GFX::Resource
{
	typedef U8 TextureUsages;
	// How texture will be accessed in pipeline
	enum TextureUsage : TextureUsages
	{
		Invalid = 0,
		PixelShader = 1,
		NonPixelShader = 2,
		All = PixelShader | NonPixelShader
	};

	// Schema of underlaying TexturePack structure allowing for texture lookup by names
	struct TextureSchema
	{
		std::unordered_map<std::string, U32> Location;
		std::unordered_map<std::string, std::pair<TextureType, TextureUsage>> Info;

		void AddTexture(const std::string& name, TextureType type, TextureUsage usage) noexcept;
	};
}