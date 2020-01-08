#include "Mesh.h"
#include "Primitives.h"
#include "GfxResources.h"
#include "Math.h"

namespace GFX::Shape
{
	Mesh::Mesh(Graphics & gfx, std::vector<std::unique_ptr<Resource::IBindable>> && binds)
		: GfxObject(false)
	{
		if (!IsStaticInit())
			AddStaticBind(std::make_unique<Resource::Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));

		for (auto & bind : binds)
		{
			if (auto indexBuffer = dynamic_cast<Resource::IndexBuffer*>(bind.get()))
			{
				AddIndexBuffer(std::unique_ptr<Resource::IndexBuffer>(indexBuffer));
				bind.release();
			}
			else
				AddBind(std::move(bind));
		}
		AddBind(std::make_unique<Resource::ConstantTransformBuffer>(gfx, *this));
	}
}
