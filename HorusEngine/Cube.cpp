#include "Cube.h"

namespace GFX::Primitive
{
	std::string Cube::GetNameSolid(const std::vector<VertexAttribute>&& attributes) noexcept
	{
		std::string name = std::string(typeid(Cube).name()) + "S";
		for (const auto& attrib : attributes)
			name += std::to_string(static_cast<unsigned char>(attrib));
		return std::move(name);
	}

	std::string Cube::GetName(const std::vector<VertexAttribute>&& attributes) noexcept
	{
		std::string name = std::string(typeid(Cube).name()) + "N";
		for (const auto& attrib : attributes)
			name += std::to_string(static_cast<unsigned char>(attrib));
		return std::move(name);
	}

	std::shared_ptr<Data::VertexLayout> Cube::GetLayoutSolid(const std::vector<VertexAttribute>&& attributes) noexcept
	{
		std::shared_ptr<Data::VertexLayout> layout = std::make_shared<Data::VertexLayout>();
		for (const auto& attrib : attributes)
			layout->Append(attrib);
		return layout;
	}

	std::shared_ptr<Data::VertexLayout> Cube::GetLayout(const std::vector<VertexAttribute>&& attributes) noexcept
	{
		std::shared_ptr<Data::VertexLayout> layout = std::make_shared<Data::VertexLayout>();
		layout->Append(VertexAttribute::Normal);
		for (const auto& attrib : attributes)
			layout->Append(attrib);
		return layout;
	}

	Data::VertexBufferData Cube::MakeSolidVertex(const std::vector<VertexAttribute>&& attributes) noexcept
	{
		constexpr float POINT = 0.5f;
		Data::VertexBufferData vertices(GetLayoutSolid(std::forward<const std::vector<VertexAttribute>>(attributes)), 8);

		vertices[0].SetByIndex(0, std::move(DirectX::XMFLOAT3(-POINT, -POINT, -POINT)));
		vertices[1].SetByIndex(0, std::move(DirectX::XMFLOAT3(-POINT, POINT, -POINT)));
		vertices[2].SetByIndex(0, std::move(DirectX::XMFLOAT3(POINT, POINT, -POINT)));
		vertices[3].SetByIndex(0, std::move(DirectX::XMFLOAT3(POINT, -POINT, -POINT)));
		vertices[4].SetByIndex(0, std::move(DirectX::XMFLOAT3(-POINT, -POINT, POINT)));
		vertices[5].SetByIndex(0, std::move(DirectX::XMFLOAT3(-POINT, POINT, POINT)));
		vertices[6].SetByIndex(0, std::move(DirectX::XMFLOAT3(POINT, POINT, POINT)));
		vertices[7].SetByIndex(0, std::move(DirectX::XMFLOAT3(POINT, -POINT, POINT)));

		return std::move(vertices);
	}

	std::vector<unsigned int> Cube::MakeSolidIndex() noexcept
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

	IndexedTriangleList Cube::MakeSolid(const std::vector<VertexAttribute>&& attributes) noexcept
	{
		return
		{
			std::move(MakeSolidVertex(std::forward<const std::vector<VertexAttribute>>(attributes))),
			std::move(MakeSolidIndex())
		};
	}

	std::vector<unsigned int> Cube::MakeSkyboxIndex() noexcept
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
			std::move(MakeSolidVertex()),
			std::move(MakeSkyboxIndex())
		};
	}

	IndexedTriangleList Cube::Make(const std::vector<VertexAttribute>&& attributes) noexcept
	{
		constexpr float point = 0.5f;
		Data::VertexBufferData vertices(GetLayout(std::forward<const std::vector<VertexAttribute>>(attributes)), 24);

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
}