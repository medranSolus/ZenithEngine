#pragma once
#include "VertexBufferData.h"

namespace GFX::Primitive
{
	class IndexedTriangleList
	{
	public:
		Data::VertexBufferData vertices;
		std::vector<unsigned int> indices;

		IndexedTriangleList() noexcept {}
		IndexedTriangleList(Data::VertexBufferData verticesIn, std::vector<unsigned int> indicesIn);
		IndexedTriangleList(const IndexedTriangleList&) = default;
		IndexedTriangleList& operator=(const IndexedTriangleList&) = default;
		~IndexedTriangleList() = default;

		void Transform(DirectX::FXMMATRIX matrix);
		void SetNormals() noexcept(!IS_DEBUG);
	};
}