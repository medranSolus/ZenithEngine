#pragma once
#include "IndexedTriangleList.h"

namespace GFX::Primitive
{
	class Cube
	{
	public:
		template<typename V>
		static IndexedTriangleList<V> Make()
		{
			constexpr float point = 0.5f;
			std::vector<DirectX::XMFLOAT3> vertices;
			vertices.emplace_back(-point, -point, -point); // 0
			vertices.emplace_back(-point, point, -point);  // 1
			vertices.emplace_back(point, point, -point);   // 2
			vertices.emplace_back(point, -point, -point);  // 3
			vertices.emplace_back(-point, -point, point);  // 4
			vertices.emplace_back(-point, point, point);   // 5
			vertices.emplace_back(point, point, point);    // 6
			vertices.emplace_back(point, -point, point);   // 7

			std::vector<V> v(8);
			for (unsigned char i = 0; i < 8; ++i)
				v.at(i).pos = vertices.at(i);
			return
			{
				std::move(v),
				{
					0,1,2, 0,2,3, // Front
					4,5,1, 4,1,0, // Left
					7,6,5, 7,5,4, // Back
					3,2,6, 3,6,7, // Right
					1,5,6, 1,6,2, // Top
					4,0,3, 4,3,7  // Down
				}
			};
		}
	};
}