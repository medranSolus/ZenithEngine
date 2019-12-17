#pragma once
#include "IndexedTriangleList.h"
#include "Vertex.h"

namespace FileService
{
	class ObjLoader
	{
	public:
		ObjLoader() = delete;
		ObjLoader(ObjLoader &) = delete;
		ObjLoader & operator=(ObjLoader &) = delete;
		~ObjLoader() = delete;

		static GFX::Primitive::IndexedTriangleList LoadMesh(const std::string & filename);
	};
}
