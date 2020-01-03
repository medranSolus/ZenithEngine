#pragma once
#include "BaseShape.h"

namespace GFX::Shape
{
	class Mesh : public BaseShape<Mesh>
	{
		mutable DirectX::XMFLOAT4X4 transform;

	public:
		Mesh(Graphics & gfx, std::vector<std::unique_ptr<Resource::IBindable>> && binds);

		DirectX::XMMATRIX GetTransformMatrix() const noexcept override { return DirectX::XMLoadFloat4x4(&transform); }

		void Draw(Graphics & gfx, const DirectX::FXMMATRIX & finalTransform) const noexcept;
	};
}
