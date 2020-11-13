#include "Mesh.h"

namespace GFX::Shape
{
	Mesh::Mesh(Graphics& gfx, GfxResPtr<Resource::IndexBuffer>&& indexBuffer,
		GfxResPtr<Resource::VertexBuffer>&& vertexBuffer, std::vector<Pipeline::Technique>&& techniques)
		: BaseShape(gfx, std::forward<GfxResPtr<Resource::IndexBuffer>&&>(indexBuffer), std::forward<GfxResPtr<Resource::VertexBuffer>&&>(vertexBuffer)),
		GfxObject(false)
	{
		SetTechniques(gfx, std::forward<std::vector<Pipeline::Technique>&&>(techniques), *this);
	}
}