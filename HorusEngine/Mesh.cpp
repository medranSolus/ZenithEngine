#include "Mesh.h"

namespace GFX::Shape
{
	Mesh::Mesh(Graphics& gfx, std::shared_ptr<Resource::IndexBuffer> indexBuffer,
		std::shared_ptr<Resource::VertexBuffer> vertexBuffer, std::vector<std::shared_ptr<Pipeline::Technique>>&& techniques)
		: BaseShape(gfx, *this, indexBuffer, vertexBuffer), GfxObject(false)
	{
		SetTechniques(std::forward<std::vector<std::shared_ptr<Pipeline::Technique>>>(techniques));
	}
}