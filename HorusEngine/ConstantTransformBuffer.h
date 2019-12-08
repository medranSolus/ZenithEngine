#pragma once
#include "ConstantVertexBuffer.h"
#include "ShaderConstantBuffers.h"
#include "IDrawable.h"
#include <DirectXMath.h>

namespace GFX::Resource
{
	class ConstantTransformBuffer : public IBindable
	{
		static std::unique_ptr<ConstantVertexBuffer<TransformConstatBuffer>> vertexBuffer;
		const Object::IDrawable & parent;

	public:
		ConstantTransformBuffer(Graphics & gfx, const Object::IDrawable & parent, UINT slot = 0U);

		void Bind(Graphics & gfx) noexcept override;
	};
}
