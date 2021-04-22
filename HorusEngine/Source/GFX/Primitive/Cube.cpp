#include "GFX/Primitive/Cube.h"

namespace GFX::Primitive
{
	std::string Cube::GetNameSolid(const std::vector<VertexAttribute>& attributes) noexcept
	{
		std::string name = "QS";
		for (const auto& attrib : attributes)
			name += std::to_string(static_cast<U8>(attrib));
		return name;
	}

	std::string Cube::GetName(const std::vector<VertexAttribute>& attributes) noexcept
	{
		std::string name = "QN";
		for (const auto& attrib : attributes)
			name += std::to_string(static_cast<U8>(attrib));
		return name;
	}

	std::shared_ptr<Data::VertexLayout> Cube::GetLayoutSolid(const std::vector<VertexAttribute>& attributes) noexcept
	{
		std::shared_ptr<Data::VertexLayout> layout = std::make_shared<Data::VertexLayout>();
		for (const auto& attrib : attributes)
			layout->Append(attrib);
		return layout;
	}

	std::shared_ptr<Data::VertexLayout> Cube::GetLayout(const std::vector<VertexAttribute>& attributes) noexcept
	{
		std::shared_ptr<Data::VertexLayout> layout = std::make_shared<Data::VertexLayout>();
		layout->Append(VertexAttribute::Normal);
		for (const auto& attrib : attributes)
			layout->Append(attrib);
		return layout;
	}

	Data::VertexBufferData Cube::MakeSolidVertex(const std::vector<VertexAttribute>& attributes) noexcept
	{
		constexpr float POINT = 0.5f;
		Data::VertexBufferData vertices(GetLayoutSolid(attributes), 8);
		vertices.SetBox({ POINT, -POINT, -POINT, POINT, -POINT, POINT });

		vertices[0].SetByIndex(0, Float3(-POINT, -POINT, -POINT));
		vertices[1].SetByIndex(0, Float3(-POINT, POINT, -POINT));
		vertices[2].SetByIndex(0, Float3(POINT, POINT, -POINT));
		vertices[3].SetByIndex(0, Float3(POINT, -POINT, -POINT));
		vertices[4].SetByIndex(0, Float3(-POINT, -POINT, POINT));
		vertices[5].SetByIndex(0, Float3(-POINT, POINT, POINT));
		vertices[6].SetByIndex(0, Float3(POINT, POINT, POINT));
		vertices[7].SetByIndex(0, Float3(POINT, -POINT, POINT));

		return vertices;
	}

	std::vector<U32> Cube::MakeSolidIndex() noexcept
	{
		return
		{
			0,1,2, 0,2,3, // Front
			4,5,1, 4,1,0, // Left
			7,6,5, 7,5,4, // Back
			3,2,6, 3,6,7, // Right
			1,5,6, 1,6,2, // Top
			4,0,3, 4,3,7  // Down
		};
	}

	IndexedTriangleList Cube::MakeSolid(const std::vector<VertexAttribute>& attributes) noexcept
	{
		return
		{
			MakeSolidVertex(attributes),
			MakeSolidIndex()
		};
	}

	std::vector<U32> Cube::MakeSkyboxIndex() noexcept
	{
		return
		{
			2,1,0, 3,2,0, // Front
			1,5,4, 0,1,4, // Left
			5,6,7, 4,5,7, // Back
			6,2,3, 7,6,3, // Right
			6,5,1, 2,6,1, // Top
			3,0,4, 7,3,4  // Down
		};
	}

	IndexedTriangleList Cube::MakeSkybox() noexcept
	{
		return
		{
			MakeSolidVertex(),
			MakeSkyboxIndex()
		};
	}

	IndexedTriangleList Cube::Make(const std::vector<VertexAttribute>& attributes) noexcept
	{
		constexpr float POINT = 0.5f;
		Data::VertexBufferData vertices(GetLayout(attributes), 24);
		vertices.SetBox({ POINT, -POINT, -POINT, POINT, -POINT, POINT });

		// Front
		vertices[0].SetByIndex(0, Float3(-POINT, -POINT, -POINT));
		vertices[1].SetByIndex(0, Float3(-POINT, POINT, -POINT));
		vertices[2].SetByIndex(0, Float3(POINT, POINT, -POINT));
		vertices[3].SetByIndex(0, Float3(POINT, -POINT, -POINT));
		// Left
		vertices[4].SetByIndex(0, Float3(-POINT, -POINT, POINT));
		vertices[5].SetByIndex(0, Float3(-POINT, POINT, POINT));
		vertices[6].SetByIndex(0, Float3(-POINT, POINT, -POINT));
		vertices[7].SetByIndex(0, Float3(-POINT, -POINT, -POINT));
		// Back
		vertices[8].SetByIndex(0, Float3(POINT, -POINT, POINT));
		vertices[9].SetByIndex(0, Float3(POINT, POINT, POINT));
		vertices[10].SetByIndex(0, Float3(-POINT, POINT, POINT));
		vertices[11].SetByIndex(0, Float3(-POINT, -POINT, POINT));
		// Right
		vertices[12].SetByIndex(0, Float3(POINT, -POINT, -POINT));
		vertices[13].SetByIndex(0, Float3(POINT, POINT, -POINT));
		vertices[14].SetByIndex(0, Float3(POINT, POINT, POINT));
		vertices[15].SetByIndex(0, Float3(POINT, -POINT, POINT));
		// Top
		vertices[16].SetByIndex(0, Float3(-POINT, POINT, -POINT));
		vertices[17].SetByIndex(0, Float3(-POINT, POINT, POINT));
		vertices[18].SetByIndex(0, Float3(POINT, POINT, POINT));
		vertices[19].SetByIndex(0, Float3(POINT, POINT, -POINT));
		// Down
		vertices[20].SetByIndex(0, Float3(-POINT, -POINT, POINT));
		vertices[21].SetByIndex(0, Float3(-POINT, -POINT, -POINT));
		vertices[22].SetByIndex(0, Float3(POINT, -POINT, -POINT));
		vertices[23].SetByIndex(0, Float3(POINT, -POINT, POINT));

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
		return list;
	}
}