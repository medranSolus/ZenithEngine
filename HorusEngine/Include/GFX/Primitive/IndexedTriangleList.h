#pragma once
#include "GFX/Data/VertexBufferData.h"

namespace GFX::Primitive
{
	class IndexedTriangleList final
	{
	public:
		Data::VertexBufferData Vertices;
		std::vector<U32> Indices;

		IndexedTriangleList() = default;
		IndexedTriangleList(Data::VertexBufferData&& vertices, std::vector<U32>&& indices) noexcept;
		IndexedTriangleList(IndexedTriangleList&&) = default;
		IndexedTriangleList(const IndexedTriangleList&) = default;
		IndexedTriangleList& operator=(IndexedTriangleList&&) = default;
		IndexedTriangleList& operator=(const IndexedTriangleList&) = default;
		~IndexedTriangleList() = default;

		void Transform(const Matrix& matrix) noexcept;
		void SetNormals() noexcept;
	};
}