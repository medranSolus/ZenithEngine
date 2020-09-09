#pragma once
#include "VertexBufferData.h"

namespace GFX::Primitive
{
	class IndexedTriangleList
	{
	public:
		Data::VertexBufferData vertices;
		std::vector<unsigned int> indices;

		inline IndexedTriangleList() noexcept {}
		IndexedTriangleList(Data::VertexBufferData verticesIn, std::vector<unsigned int> indicesIn) noexcept(!IS_DEBUG);
		IndexedTriangleList(const IndexedTriangleList&) = default;
		IndexedTriangleList& operator=(const IndexedTriangleList&) = default;
		~IndexedTriangleList() = default;

		void Transform(DirectX::FXMMATRIX matrix) noexcept(!IS_DEBUG);
		void SetNormals() noexcept(!IS_DEBUG);
	};
}