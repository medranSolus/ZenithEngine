#pragma once
#include "IndexedTriangleList.h"

namespace GFX::Primitive
{
	class Square
	{
	public:
		Square() = delete;

		static IndexedTriangleList Make(const std::vector<VertexAttribute> && attributes = {})
		{
			constexpr float point = 0.5f;
			BasicType::VertexLayout layout;
			for (const auto & attrib : attributes)
				layout.Append(attrib);
			BasicType::VertexDataBuffer vertices(std::move(layout), 4);
			vertices[0].SetByIndex(0, std::move(DirectX::XMFLOAT3(-point, -point, 0.0f)));
			vertices[1].SetByIndex(0, std::move(DirectX::XMFLOAT3(-point, point, 0.0f)));
			vertices[2].SetByIndex(0, std::move(DirectX::XMFLOAT3(point, point, 0.0f)));
			vertices[3].SetByIndex(0, std::move(DirectX::XMFLOAT3(point, -point, 0.0f)));

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
