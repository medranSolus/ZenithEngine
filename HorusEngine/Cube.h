#pragma once
#include "IndexedTriangleList.h"

namespace GFX::Primitive
{
	class Cube
	{
	public:
		Cube() = delete;

		static IndexedTriangleList MakeSolid(const std::vector<VertexAttribute>&& attributes = {});
		static IndexedTriangleList Make(const std::vector<VertexAttribute>&& attributes = {});
	};
}