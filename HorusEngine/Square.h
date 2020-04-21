#pragma once
#include "IndexedTriangleList.h"

namespace GFX::Primitive
{
	class Square
	{
	public:
		Square() = delete;

		static IndexedTriangleList Make(const std::vector<VertexAttribute>&& attributes = {});
	};
}