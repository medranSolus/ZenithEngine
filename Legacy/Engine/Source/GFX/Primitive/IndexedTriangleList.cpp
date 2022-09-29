#include "GFX/Primitive/IndexedTriangleList.h"

namespace ZE::GFX::Primitive
{
	IndexedTriangleList::IndexedTriangleList(Data::VertexBufferData&& vertices, std::vector<U32>&& indices) noexcept
		: Vertices(std::move(vertices)), Indices(std::move(indices))
	{
		assert(Vertices.Size() > 2);
		assert(Indices.size() % 3 == 0);
	}

	void IndexedTriangleList::Transform(const Matrix& matrix) noexcept
	{
		for (U64 i = 0, size = Vertices.Size(); i < size; ++i)
		{
			Math::XMStoreFloat3(&Vertices[i].Get<VertexAttribute::Position3D>(),
				Math::XMVector3Transform(Math::XMLoadFloat3(&Vertices[i].Get<VertexAttribute::Position3D>()), matrix));
		}
	}

	void IndexedTriangleList::SetNormals() noexcept
	{
		assert(Indices.size() % 3 == 0 && Indices.size() > 0);
		for (U64 i = 0; i < Indices.size(); i += 3)
		{
			Data::Vertex v0 = Vertices[Indices.at(i)];
			Data::Vertex v1 = Vertices[Indices.at(i + 1)];
			Data::Vertex v2 = Vertices[Indices.at(i + 2)];
			const auto& p0 = Math::XMLoadFloat3(&v0.Get<VertexAttribute::Position3D>());

			const Vector normal = Math::XMVector3Normalize(Math::XMVector3Cross(
				Math::XMVectorSubtract(Math::XMLoadFloat3(&v1.Get<VertexAttribute::Position3D>()), p0),
				Math::XMVectorSubtract(Math::XMLoadFloat3(&v2.Get<VertexAttribute::Position3D>()), p0)));
			Math::XMStoreFloat3(&v0.Get<VertexAttribute::Normal>(), normal);
			Math::XMStoreFloat3(&v1.Get<VertexAttribute::Normal>(), normal);
			Math::XMStoreFloat3(&v2.Get<VertexAttribute::Normal>(), normal);
		}
	}
}