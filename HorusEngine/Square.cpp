#include "Square.h"

namespace GFX::Primitive
{
	IndexedTriangleList Square::Make(const std::vector<VertexAttribute>&& attributes)
	{
		constexpr float point = 0.5f;
		std::shared_ptr<Data::VertexLayout> layout = std::make_shared<Data::VertexLayout>();
		for (const auto& attrib : attributes)
			layout->Append(attrib);
		Data::VertexBufferData vertices(layout, 4);
		vertices[0].SetByIndex(0, std::move(DirectX::XMFLOAT3(-point, -point, 0.0f)));
		vertices[1].SetByIndex(0, std::move(DirectX::XMFLOAT3(-point, point, 0.0f)));
		vertices[2].SetByIndex(0, std::move(DirectX::XMFLOAT3(point, point, 0.0f)));
		vertices[3].SetByIndex(0, std::move(DirectX::XMFLOAT3(point, -point, 0.0f)));

		return
		{
			std::move(vertices),
			{
				0,1,2, 0,2,3,
			}, typeid(Square).name()
		};
	}
}