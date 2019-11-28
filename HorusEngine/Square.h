#pragma once
#include "IndexedTriangleList.h"

namespace GFX::Primitive
{
	class Square
	{
	public:
		template<typename V>
		static IndexedTriangleList<V> Make()
		{
			constexpr float point = 0.5f;
			std::vector<DirectX::XMFLOAT3> vertices;
			vertices.emplace_back(-point, -point, 1.0f); // 0
			vertices.emplace_back(-point, point, 1.0f);  // 1
			vertices.emplace_back(point, point, 1.0f);   // 2
			vertices.emplace_back(point, -point, 1.0f);  // 3

			std::vector<V> v(4);
			for (unsigned char i = 0; i < 4; ++i)
				v.at(i).pos = vertices.at(i);
			return
			{
				std::move(v),
				{
					0,1,2, 0,2,3,
				}
			};
		}
	};
}
