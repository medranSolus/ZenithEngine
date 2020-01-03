#pragma once
#include "ConstantVertexBuffer.h"
#include "ShaderConstantBuffers.h"
#include "IShape.h"
#include <DirectXMath.h>

namespace GFX::Resource
{
	class ConstantTransformBuffer : public IBindable
	{
		static std::unique_ptr<ConstantVertexBuffer<TransformConstatBuffer>> vertexBuffer;
		const Shape::IShape & parent;

	public:
		ConstantTransformBuffer(Graphics & gfx, const Shape::IShape & parent, UINT slot = 0U);

		void Bind(Graphics & gfx) noexcept override;
	};
}
