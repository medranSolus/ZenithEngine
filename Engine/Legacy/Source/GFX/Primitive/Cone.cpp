#include "GFX/Primitive/Cone.h"

namespace ZE::GFX::Primitive
{
	std::string Cone::GetNameSolid(U32 density, const std::vector<VertexAttribute>& attributes) noexcept
	{
		std::string name = "CS";
		for (const auto& attrib : attributes)
			name += std::to_string(static_cast<U8>(attrib));
		if (!density)
			density = 1;
		density *= 3;
		return name + std::to_string(density);
	}

	std::shared_ptr<Data::VertexLayout> Cone::GetLayoutSolid(const std::vector<VertexAttribute>& attributes) noexcept
	{
		std::shared_ptr<Data::VertexLayout> layout = std::make_shared<Data::VertexLayout>();
		for (const auto& attrib : attributes)
			layout->Append(attrib);
		return layout;
	}

	IndexedTriangleList Cone::MakeSolid(U32 density, const std::vector<VertexAttribute>& attributes) noexcept
	{
		if (!density)
			density = 1;
		density *= 3;

		Data::VertexBufferData vertices(GetLayoutSolid(attributes), static_cast<U64>(density) + 1);
		vertices.SetBox({ 1.0f, 0.0f, -1.0f, 1.0f, -1.0f, 1.0f });
		vertices[0].SetByIndex(0, Float3(0.0f, 1.0f, 0.0f));
		vertices[1].SetByIndex(0, Float3(0.0f, 0.0f, 1.0f)); // Base of circle

		const float angle = 2.0f * static_cast<float>(M_PI / density);
		const Vector base = Math::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

		for (U64 i = 1; i < density; ++i)
		{
			Math::XMStoreFloat3(&vertices[i + 1].Get<VertexAttribute::Position3D>(),
				Math::XMVector3Transform(base, Math::XMMatrixRotationY(angle * i)));
		}

		std::vector<U32> indices;
		indices.reserve(static_cast<U64>(density - 1) * 6);
		for (U32 i = 2; i < density; ++i)
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