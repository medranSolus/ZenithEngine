#include "Mesh.h"
#include "GfxResources.h"
#include "VertexDataBuffer.h"
#include "Primitives.h"
#include "Math.h"

namespace GFX::Shape
{
	Mesh::Mesh(Graphics & gfx, std::vector<std::unique_ptr<Resource::IBindable>> && binds)
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

	void Mesh::Draw(Graphics & gfx, const DirectX::FXMMATRIX & finalTransform) const noexcept
	{
		DirectX::XMStoreFloat4x4(&transform, finalTransform);
		IShape::Draw(gfx);
	}
}
