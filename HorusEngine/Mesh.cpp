#include "Mesh.h"
#include "Primitives.h"
#include "GfxResources.h"
#include "Math.h"

namespace GFX::Shape
{
	Mesh::Mesh(Graphics & gfx, std::vector<std::shared_ptr<Resource::IBindable>> && binds)
		: GfxObject(false)
	{
		AddBind(std::make_shared<Resource::Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));

		for (auto & bind : binds)
			AddBind(bind);
		AddBind(std::make_shared<Resource::ConstantTransformBuffer>(gfx, *this));
	}
}
