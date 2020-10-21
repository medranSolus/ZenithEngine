#include "Cone.h"
#include "Math.h"

namespace GFX::Primitive
{
	std::string Cone::GetNameSolid(unsigned int density, const std::vector<VertexAttribute>& attributes) noexcept
	{
		std::string name = std::string(typeid(Cone).name());
		for (const auto& attrib : attributes)
			name += std::to_string(static_cast<unsigned char>(attrib));
		if (!density)
			density = 1;
		density *= 3;
		return std::move(name + std::to_string(density));
	}

	std::shared_ptr<Data::VertexLayout> Cone::GetLayoutSolid(const std::vector<VertexAttribute>& attributes) noexcept
	{
		std::shared_ptr<Data::VertexLayout> layout = std::make_shared<Data::VertexLayout>();
		for (const auto& attrib : attributes)
			layout->Append(attrib);
		return layout;
	}

	IndexedTriangleList Cone::MakeSolid(unsigned int density, const std::vector<VertexAttribute>& attributes) noexcept
	{
		if (!density)
			density = 1;
		density *= 3;

		Data::VertexBufferData vertices(GetLayoutSolid(attributes), static_cast<size_t>(density) + 1);
		vertices[0].SetByIndex(0, std::move(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f))); // Non moveable center
		vertices[1].SetByIndex(0, std::move(DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f))); // Base of circle

		const float angle = 2.0f * static_cast<float>(M_PI / density);
		const DirectX::XMVECTOR base = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

		for (size_t i = 1; i < density; ++i)
			DirectX::XMStoreFloat3(&vertices[i + 1].Get<VertexAttribute::Position3D>(),
				DirectX::XMVector3Transform(base, DirectX::XMMatrixRotationY(angle * i)));

		std::vector<unsigned int> indices;
		indices.reserve(static_cast<size_t>(density - 2) * 6 + 3);
		for (unsigned int i = 2; i < density; ++i)
		{
			// Cone wall
			indices.emplace_back(0);
			indices.emplace_back(i);
			indices.emplace_back(i + 1);
			// Cone base
			indices.emplace_back(1);
			indices.emplace_back(i + 1);
			indices.emplace_back(i);
		}
		// First wall
		indices.emplace_back(0);
		indices.emplace_back(1);
		indices.emplace_back(2);
		// Last wall
		indices.emplace_back(0);
		indices.emplace_back(density);
		indices.emplace_back(1);

		return { std::move(vertices), std::move(indices) };
	}
}