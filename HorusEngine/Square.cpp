#include "Square.h"

namespace GFX::Primitive
{
	std::string Square::GetName(const std::vector<VertexAttribute>&& attributes) noexcept
	{
		std::string name = std::string(typeid(Square).name());
		for (const auto& attrib : attributes)
			name += std::to_string(static_cast<unsigned char>(attrib));
		return std::move(name);
	}

	std::shared_ptr<Data::VertexLayout> Square::GetLayout(const std::vector<VertexAttribute>&& attributes) noexcept
	{
		std::shared_ptr<Data::VertexLayout> layout = std::make_shared<Data::VertexLayout>();
		for (const auto& attrib : attributes)
			layout->Append(attrib);
		return layout;
	}

	IndexedTriangleList Square::Make(const std::vector<VertexAttribute>&& attributes)
	{
		constexpr float point = 0.5f;
		Data::VertexBufferData vertices(GetLayout(std::forward<const std::vector<VertexAttribute>>(attributes)), 4);

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
}