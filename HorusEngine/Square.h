#pragma once
#include "IndexedTriangleList.h"

namespace GFX::Primitive
{
	class Square
	{
	public:
		static IndexedTriangleList Make()
		{
			constexpr float point = 0.5f;
			BasicType::VertexDataBuffer vertices(std::move(BasicType::VertexLayout{}.Append(VertexAttribute::Position3D)), 4);
			vertices.EmplaceBack(DirectX::XMFLOAT3(-point, -point, 0.0f),
				DirectX::XMFLOAT3(-point, point, 0.0f),
				DirectX::XMFLOAT3(point, point, 0.0f),
				DirectX::XMFLOAT3(point, -point, 0.0f));

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
