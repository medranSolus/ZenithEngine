#pragma once
#include "IndexedTriangleList.h"

namespace GFX::Primitive
{
	class Cube
	{
	public:
		Cube() = delete;

		static IndexedTriangleList MakeSolid(const std::vector<VertexAttribute> && attributes = {})
		{
			constexpr float point = 0.5f;
			BasicType::VertexLayout layout;
			for (const auto & attrib : attributes)
				layout.Append(attrib);
			BasicType::VertexDataBuffer vertices(std::move(layout), 8);

			vertices[0].SetByIndex(0, std::move(DirectX::XMFLOAT3(-point, -point, -point)));
			vertices[1].SetByIndex(0, std::move(DirectX::XMFLOAT3(-point, point, -point)));
			vertices[2].SetByIndex(0, std::move(DirectX::XMFLOAT3(point, point, -point)));
			vertices[3].SetByIndex(0, std::move(DirectX::XMFLOAT3(point, -point, -point)));
			vertices[4].SetByIndex(0, std::move(DirectX::XMFLOAT3(-point, -point, point)));
			vertices[5].SetByIndex(0, std::move(DirectX::XMFLOAT3(-point, point, point)));
			vertices[6].SetByIndex(0, std::move(DirectX::XMFLOAT3(point, point, point)));
			vertices[7].SetByIndex(0, std::move(DirectX::XMFLOAT3(point, -point, point)));

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

		static IndexedTriangleList Make(const std::vector<VertexAttribute> && attributes = {})
		{
			constexpr float point = 0.5f;
			BasicType::VertexLayout layout;
			layout.Append(VertexAttribute::Normal);
			for (const auto & attrib : attributes)
				layout.Append(attrib);
			BasicType::VertexDataBuffer vertices(std::move(layout), 24);

			// Front
			vertices[0].SetByIndex(0, std::move(DirectX::XMFLOAT3(-point, -point, -point)));
			vertices[1].SetByIndex(0, std::move(DirectX::XMFLOAT3(-point, point, -point)));
			vertices[2].SetByIndex(0, std::move(DirectX::XMFLOAT3(point, point, -point)));
			vertices[3].SetByIndex(0, std::move(DirectX::XMFLOAT3(point, -point, -point)));
			// Left
			vertices[4].SetByIndex(0, std::move(DirectX::XMFLOAT3(-point, -point, point)));
			vertices[5].SetByIndex(0, std::move(DirectX::XMFLOAT3(-point, point, point)));
			vertices[6].SetByIndex(0, std::move(DirectX::XMFLOAT3(-point, point, -point)));
			vertices[7].SetByIndex(0, std::move(DirectX::XMFLOAT3(-point, -point, -point)));
			// Back
			vertices[8].SetByIndex(0, std::move(DirectX::XMFLOAT3(point, -point, point)));
			vertices[9].SetByIndex(0, std::move(DirectX::XMFLOAT3(point, point, point)));
			vertices[10].SetByIndex(0, std::move(DirectX::XMFLOAT3(-point, point, point)));
			vertices[11].SetByIndex(0, std::move(DirectX::XMFLOAT3(-point, -point, point)));
			// Right
			vertices[12].SetByIndex(0, std::move(DirectX::XMFLOAT3(point, -point, -point)));
			vertices[13].SetByIndex(0, std::move(DirectX::XMFLOAT3(point, point, -point)));
			vertices[14].SetByIndex(0, std::move(DirectX::XMFLOAT3(point, point, point)));
			vertices[15].SetByIndex(0, std::move(DirectX::XMFLOAT3(point, -point, point)));
			// Top
			vertices[16].SetByIndex(0, std::move(DirectX::XMFLOAT3(-point, point, -point)));
			vertices[17].SetByIndex(0, std::move(DirectX::XMFLOAT3(-point, point, point)));
			vertices[18].SetByIndex(0, std::move(DirectX::XMFLOAT3(point, point, point)));
			vertices[19].SetByIndex(0, std::move(DirectX::XMFLOAT3(point, point, -point)));
			// Down	
			vertices[20].SetByIndex(0, std::move(DirectX::XMFLOAT3(-point, -point, point)));
			vertices[21].SetByIndex(0, std::move(DirectX::XMFLOAT3(-point, -point, -point)));
			vertices[22].SetByIndex(0, std::move(DirectX::XMFLOAT3(point, -point, -point)));
			vertices[23].SetByIndex(0, std::move(DirectX::XMFLOAT3(point, -point, point)));

			IndexedTriangleList list =
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