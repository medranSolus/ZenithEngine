#pragma once
#include "IndexedTriangleList.h"

namespace GFX::Primitive
{
	class Cube
	{
	public:
		template<typename V>
		static IndexedTriangleList<V> MakeSolid()
		{
			constexpr float point = 0.5f;
			std::vector<V> vertices(8);
			vertices[0].pos = { -point, -point, -point };
			vertices[1].pos = { -point, point, -point };
			vertices[2].pos = { point, point, -point };
			vertices[3].pos = { point, -point, -point };
			vertices[4].pos = { -point, -point, point };
			vertices[5].pos = { -point, point, point };
			vertices[6].pos = { point, point, point };
			vertices[7].pos = { point, -point, point };

			return
			{
				std::move(vertices),
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

		template<typename V>
		static IndexedTriangleList<V> Make()
		{
			constexpr float point = 0.5f;
			std::vector<V> vertices(24);
			// Front
			vertices[0].pos = { -point,-point,-point };
			vertices[1].pos = { -point,point,-point };
			vertices[2].pos = { point,point,-point };
			vertices[3].pos = { point,-point,-point };
			// Left
			vertices[4].pos = { -point,-point,point };
			vertices[5].pos = { -point,point,point };
			vertices[6].pos = { -point,point,-point };
			vertices[7].pos = { -point,-point,-point };
			// Back
			vertices[8].pos = { point,-point,point };
			vertices[9].pos = { point,point,point };
			vertices[10].pos = { -point,point,point };
			vertices[11].pos = { -point,-point,point };
			// Right
			vertices[12].pos = { point,-point,-point };
			vertices[13].pos = { point,point,-point };
			vertices[14].pos = { point,point,point };
			vertices[15].pos = { point,-point,point };
			// Top
			vertices[16].pos = { -point,point,-point };
			vertices[17].pos = { -point,point,point };
			vertices[18].pos = { point,point,point };
			vertices[19].pos = { point,point,-point };
			// Down	
			vertices[20].pos = { -point,-point,point };
			vertices[21].pos = { -point,-point,-point };
			vertices[22].pos = { point,-point,-point };
			vertices[23].pos = { point,-point,point };

			IndexedTriangleList<V> list =
			{
				std::move(vertices),
				{
					0,1,2,    0,2,3,    // Front
					4,5,6,    4,6,7,    // Left
					8,9,10,   8,10,11,  // Back
					12,13,14, 12,14,15, // Right
					16,17,18, 16,18,19, // Top
					20,21,22, 20,22,23  // Down
				}
			};
			list.SetNormals();
			return std::move(list);
		}
	};
}