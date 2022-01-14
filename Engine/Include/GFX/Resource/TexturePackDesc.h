#pragma once
#include "GFX/Surface.h"
#include "TextureSchema.h"

namespace ZE::GFX::Resource
{
	// Info of how single texture should be created
	struct TextureDesc
	{
		TextureType Type;
		TextureUsages Usage;
		std::vector<Surface> Surfaces;
	};

	// Describes set of textures to create pack with
	struct TexturePackDesc
	{
		std::vector<TextureDesc> Textures;

		void Init(const TextureSchema& schema) noexcept;
		void AddTexture(const TextureSchema& schema, const std::string& name, std::vector<Surface>&& surfaces) noexcept;
		void AddTexture(TextureType type, TextureUsage usage, std::vector<Surface>&& surfaces) noexcept;
	};
}