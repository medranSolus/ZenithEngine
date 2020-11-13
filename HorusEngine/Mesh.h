#pragma once
#include "BaseShape.h"

namespace GFX::Shape
{
	class Mesh : public BaseShape, public GfxObject
	{
	public:
		Mesh(Graphics& gfx, GfxResPtr<Resource::IndexBuffer>&& indexBuffer,
			GfxResPtr<Resource::VertexBuffer>&& vertexBuffer, std::vector<Pipeline::Technique>&& techniques);
		virtual ~Mesh() = default;
	};
}