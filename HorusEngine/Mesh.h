#pragma once
#include "BaseShape.h"

namespace GFX::Shape
{
	class Mesh : public BaseShape, public GfxObject
	{
		const std::string* name;

	public:
		Mesh(Graphics& gfx, const std::string& name, GfxResPtr<Resource::IndexBuffer>&& indexBuffer,
			GfxResPtr<Resource::VertexBuffer>&& vertexBuffer, std::vector<Pipeline::Technique>&& techniques);
		virtual ~Mesh() = default;

		inline const std::string& GetName() const noexcept { return *name; }
	};
}