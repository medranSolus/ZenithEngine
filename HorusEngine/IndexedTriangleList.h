#pragma once
#define _USE_MATH_DEFINES
#include <DirectXMath.h>
#include <vector>

namespace GFX::Primitive
{
	// Requires type to have public member "pos" with 3 floats
	template<typename V>
	class IndexedTriangleList
	{
	public:
		std::vector<V> vertices;
		std::vector<unsigned int> indices;

		IndexedTriangleList() = default;
		IndexedTriangleList(std::vector<V> verticesIn, std::vector<unsigned int> indicesIn)
			: vertices(std::move(verticesIn)), indices(std::move(indicesIn))
		{
			assert(vertices.size() > 2);
			assert(indices.size() % 3 == 0);
		}

		void Transform(DirectX::FXMMATRIX matrix)
		{
			for (auto & vertex : vertices)
			{
				const DirectX::XMVector pos = DirectX::XMLoadFloat3(&vertex.pos);
				DirectX::XMStoreFloat3(&vertex.pos, DirectX::XMVector3Transform(pos, matrix));
			}
		}

		void SetNormals() noexcept(!IS_DEBUG)
		{
			assert(indices.size() % 3 == 0 && indices.size() > 0);
			for (size_t i = 0; i < indices.size(); i += 3)
			{
				V & v0 = vertices.at(indices.at(i));
				V & v1 = vertices.at(indices.at(i + 1));
				V & v2 = vertices.at(indices.at(i + 2));
				const auto & p0 = DirectX::XMLoadFloat3(&v0.pos);

				const auto normal = DirectX::XMVector3Normalize(DirectX::XMVector3Cross((DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&v1.pos), p0)),
					DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&v2.pos), p0)));
				DirectX::XMStoreFloat3(&v0.normal, normal);
				DirectX::XMStoreFloat3(&v1.normal, normal);
				DirectX::XMStoreFloat3(&v2.normal, normal);
			}
		}
	};
}
