#pragma once
#include "BaseShape.h"

namespace ZE::GFX::Shape
{
	class Mesh : public BaseShape, public GfxObject
	{
		const std::string* name;

	public:
		Mesh(Graphics& gfx, const std::string& name, GfxResPtr<Resource::IndexBuffer>&& indexBuffer,
			GfxResPtr<Resource::VertexBuffer>&& vertexBuffer, std::vector<Pipeline::Technique>&& techniques);
		virtual ~Mesh() = default;

		constexpr const std::string& GetName() const noexcept { return *name; }
	};
}