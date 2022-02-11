#pragma once
#include "GFX/Surface.h"
#include "Schema.h"

namespace ZE::GFX::Resource::Texture
{
	// Info of how single texture should be created
	struct Desc
	{
		Type Type;
		Usages Usage;
		std::vector<Surface> Surfaces;
	};

	typedef U8 PackOptions;
	enum PackOption : PackOptions
	{
		// Textures are created as static pipeline resources during engine initializaion
		StaticCreation = 1
	};

	// Describes set of textures to create pack with
	struct PackDesc
	{
		PackOptions Options = 0;
		std::vector<Desc> Textures;

		void Init(const Schema& schema) noexcept;
		void AddTexture(const Schema& schema, const std::string& name, std::vector<Surface>&& surfaces) noexcept;
		void AddTexture(Type type, Usage usage, std::vector<Surface>&& surfaces) noexcept;
	};
}