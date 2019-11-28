#include "ObjLoader.h"

namespace FileService
{
	GFX::Primitive::IndexedTriangleList<GFX::Primitive::Vertex> ObjLoader::LoadMesh(const std::string & filename)
	{
		return GFX::Primitive::IndexedTriangleList<GFX::Primitive::Vertex>();
	}

	GFX::Primitive::IndexedTriangleList<GFX::Primitive::VertexTexture> ObjLoader::LoadTextured(const std::string & filename)
	{
		return GFX::Primitive::IndexedTriangleList<GFX::Primitive::VertexTexture>();
	}
}
