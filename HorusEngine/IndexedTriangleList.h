#pragma once
#define _USE_MATH_DEFINES
#include "VertexDataBuffer.h"
#include <DirectXMath.h>

namespace GFX::Primitive
{
	class IndexedTriangleList
	{
	public:
		BasicType::VertexDataBuffer vertices;
		std::vector<unsigned int> indices;

		IndexedTriangleList() = default;
		IndexedTriangleList(BasicType::VertexDataBuffer verticesIn, std::vector<unsigned int> indicesIn);

		void Transform(DirectX::FXMMATRIX matrix);
		void SetNormals() noexcept(!IS_DEBUG);
	};
}
