#include "Mesh.h"
#include "Primitives.h"
#include "GfxResources.h"
#include "Math.h"

namespace GFX::Shape
{
	Mesh::Mesh(Graphics & gfx, std::vector<std::shared_ptr<Resource::IBindable>> && binds)
		: GfxObject(false)
	{
		AddBind(Resource::Topology::Get(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
		AddBind(std::make_shared<Resource::ConstantTransformBuffer>(gfx, *this));
		for (auto & bind : binds)
			AddBind(bind);
	}
}
