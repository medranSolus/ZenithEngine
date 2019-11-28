#pragma once
#include "ConstantVertexBuffer.h"
#include "IDrawable.h"
#include <DirectXMath.h>

namespace GFX::Resource
{
	class ConstantTransformBuffer : public IBindable
	{
		static std::unique_ptr<ConstantVertexBuffer<DirectX::XMMATRIX>> vertexBuffer;
		const Object::IDrawable & parent;

	public:
		ConstantTransformBuffer(Graphics & gfx, const Object::IDrawable & parent);

		void Bind(Graphics & gfx) noexcept override;
	};
}
