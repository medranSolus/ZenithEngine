#pragma once
#include "BaseShape.h"

namespace GFX::Shape
{
	class Mesh : public BaseShape, public GfxObject
	{
	public:
		Mesh(Graphics& gfx, std::shared_ptr<Resource::IndexBuffer> indexBuffer,
			std::shared_ptr<Resource::VertexBuffer> vertexBuffer, std::vector<Pipeline::Technique>&& techniques);
		virtual ~Mesh() = default;
	};
}