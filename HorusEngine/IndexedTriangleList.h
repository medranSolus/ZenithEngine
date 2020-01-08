#pragma once
#include "VertexDataBuffer.h"

namespace GFX::Primitive
{
	class IndexedTriangleList
	{
	public:
		BasicType::VertexDataBuffer vertices;
		std::vector<unsigned int> indices;

		IndexedTriangleList() = default;
		IndexedTriangleList(BasicType::VertexDataBuffer verticesIn, std::vector<unsigned int> indicesIn);
		IndexedTriangleList(const IndexedTriangleList&) = default;
		IndexedTriangleList & operator=(const IndexedTriangleList&) = default;
		~IndexedTriangleList() = default;

		void Transform(DirectX::FXMMATRIX matrix);
		void SetNormals() noexcept(!IS_DEBUG);
	};
}
