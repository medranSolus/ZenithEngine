#include "Square.h"

namespace GFX::Primitive
{
	std::string Square::GetName(const std::vector<VertexAttribute>& attributes) noexcept
	{
		std::string name = std::string(typeid(Square).name());
		for (const auto& attrib : attributes)
			name += std::to_string(static_cast<unsigned char>(attrib));
		return std::move(name);
	}

	std::shared_ptr<Data::VertexLayout> Square::GetLayout(const std::vector<VertexAttribute>& attributes) noexcept
	{
		std::shared_ptr<Data::VertexLayout> layout = std::make_shared<Data::VertexLayout>();
		for (const auto& attrib : attributes)
			layout->Append(attrib);
		return layout;
	}

	std::shared_ptr<Data::VertexLayout> Square::GetLayoutNDC2D() noexcept
	{
		std::shared_ptr<Data::VertexLayout> layout = std::make_shared<Data::VertexLayout>(false);
		layout->Append(VertexAttribute::Position2D);
		return layout;
	}

	IndexedTriangleList Square::Make(const std::vector<VertexAttribute>& attributes) noexcept
	{
		constexpr float point = 0.5f;
		Data::VertexBufferData vertices(GetLayout(attributes), 4);
		vertices.SetBox({ point, -point, -point, point, 0.0f, 0.0f });

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

	IndexedTriangleList Square::MakeNDC2D() noexcept
	{
		Data::VertexBufferData vertices(GetLayoutNDC2D(), 4);
		vertices.SetBox({ 1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f });

		vertices[0].SetByIndex(0, std::move(DirectX::XMFLOAT2(-1.0f, 1.0f)));
		vertices[1].SetByIndex(0, std::move(DirectX::XMFLOAT2(1.0f, 1.0f)));
		vertices[2].SetByIndex(0, std::move(DirectX::XMFLOAT2(-1.0f, -1.0f)));
		vertices[3].SetByIndex(0, std::move(DirectX::XMFLOAT2(1.0f, -1.0f)));

		return
		{
			std::move(vertices),
			{
				0,1,2, 1,3,2
			}
		};
	}
}