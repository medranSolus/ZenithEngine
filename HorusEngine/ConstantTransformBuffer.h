#pragma once
#include "ConstantVertexBuffer.h"
#include "IDrawable.h"
#include <DirectXMath.h>

namespace GFX::Resource
{
	class ConstantTransformBuffer : public IBindable
	{
		struct Buffer
		{
			DirectX::XMMATRIX model;
			DirectX::XMMATRIX modelViewProjection;
		};

		static std::unique_ptr<ConstantVertexBuffer<Buffer>> vertexBuffer;
		const Object::IDrawable & parent;

	public:
		ConstantTransformBuffer(Graphics & gfx, const Object::IDrawable & parent);

		void Bind(Graphics & gfx) noexcept override;
	};
}
