#pragma once
#include "GFX/Resource/Texture/Schema.h"
#include "GFX/Resource/IndexBuffer.h"
#include "GFX/Resource/VertexBuffer.h"
#include "Entity.h"

namespace ZE::Data
{
	// Component storing data about single geometry
	struct Geometry
	{
		GFX::Resource::VertexBuffer Vertices;
		GFX::Resource::IndexBuffer Indices;
	};

	// Identifier of single geometry data
	struct MeshID { EID ID; };

#pragma region Functions
#ifdef _ZE_MODEL_LOADING
	// Loads model from file along with all of it's resources and places it's structure at `root` entity.
	// Root entity should already contain Transform and TransformGlobal components
	void LoadGeometryFromModel(GFX::Device& dev, GFX::Resource::Texture::Library& textureLib,
		Storage& registry, Storage& resourceRegistry, EID root, const std::string& file);
#endif
#pragma endregion
}