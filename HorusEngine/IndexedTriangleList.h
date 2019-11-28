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
	};
}