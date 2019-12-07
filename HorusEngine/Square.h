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
			std::vector<V> vertices(4);
			vertices[0].pos = { -point, -point, 0.0f };
			vertices[1].pos = { -point, point, 0.0f };
			vertices[2].pos = { point, point, 0.0f };
			vertices[3].pos = { point, -point, 0.0f };

			return
			{
				std::move(vertices),
				{
					0,1,2, 0,2,3,
				}
			};
		}
	};
}
