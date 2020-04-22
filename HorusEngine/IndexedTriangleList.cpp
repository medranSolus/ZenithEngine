#include "IndexedTriangleList.h"

namespace GFX::Primitive
{
	IndexedTriangleList::IndexedTriangleList(Data::VertexBufferData verticesIn, std::vector<unsigned int> indicesIn)
		: vertices(std::move(verticesIn)), indices(std::move(indicesIn))
	{
		assert(vertices.Size() > 2);
		assert(indices.size() % 3 == 0);
	}

	void IndexedTriangleList::Transform(DirectX::FXMMATRIX matrix)
	{
		for (size_t i = 0, size = vertices.Size(); i < size; ++i)
			DirectX::XMStoreFloat3(&vertices[i].Get<VertexAttribute::Position3D>(),
				DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&vertices[i].Get<VertexAttribute::Position3D>()), matrix));
	}

	void IndexedTriangleList::SetNormals() noexcept(!IS_DEBUG)
	{
		assert(indices.size() % 3 == 0 && indices.size() > 0);
		for (size_t i = 0; i < indices.size(); i += 3)
		{
			Data::Vertex v0 = vertices[indices.at(i)];
			Data::Vertex v1 = vertices[indices.at(i + 1)];
			Data::Vertex v2 = vertices[indices.at(i + 2)];
			const auto& p0 = DirectX::XMLoadFloat3(&v0.Get<VertexAttribute::Position3D>());

			const auto normal = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(
				DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&v1.Get<VertexAttribute::Position3D>()), p0),
				DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&v2.Get<VertexAttribute::Position3D>()), p0)));
			DirectX::XMStoreFloat3(&v0.Get<VertexAttribute::Normal>(), normal);
			DirectX::XMStoreFloat3(&v1.Get<VertexAttribute::Normal>(), normal);
			DirectX::XMStoreFloat3(&v2.Get<VertexAttribute::Normal>(), normal);
		}
	}
}