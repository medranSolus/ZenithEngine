#include "Mesh.h"

namespace GFX::Shape
{
	Mesh::Mesh(Graphics& gfx, std::shared_ptr<Resource::IndexBuffer> indexBuffer,
		std::shared_ptr<Resource::VertexBuffer> vertexBuffer, std::vector<std::shared_ptr<Pipeline::Technique>>&& techniques)
		: BaseShape(gfx, indexBuffer, vertexBuffer), GfxObject(false)
	{
		SetTechniques(gfx, std::forward<std::vector<std::shared_ptr<Pipeline::Technique>>>(techniques), *this);
	}
}