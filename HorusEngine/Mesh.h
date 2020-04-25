#pragma once
#include "BaseShape.h"
#include "GfxObject.h"

namespace GFX::Shape
{
	class Mesh : public BaseShape, public GfxObject
	{
	public:
		Mesh(Graphics& gfx, std::shared_ptr<Resource::IndexBuffer> indexBuffer,
			std::shared_ptr<Resource::VertexBuffer> vertexBuffer, std::vector<std::shared_ptr<Pipeline::Technique>>&& techniques);
		Mesh(const Mesh&) = delete;
		Mesh& operator=(const Mesh&) = delete;
		virtual ~Mesh() = default;
	};
}