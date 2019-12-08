#include "ConstantTransformBuffer.h"

namespace GFX::Resource
{
	ConstantTransformBuffer::ConstantTransformBuffer(Graphics & gfx, const Object::IDrawable & parent, UINT slot) : parent(parent)
	{
		if (!vertexBuffer)
			vertexBuffer = std::make_unique<ConstantVertexBuffer<TransformConstatBuffer>>(gfx, slot);
	}

	void ConstantTransformBuffer::Bind(Graphics & gfx) noexcept
	{
		const auto modelView = parent.GetTransformMatrix() * gfx.GetCamera();
		vertexBuffer->Update(gfx,
			{
				modelView,
				std::move(modelView * gfx.GetProjection())
			});
		vertexBuffer->Bind(gfx);
	}

	std::unique_ptr<ConstantVertexBuffer<TransformConstatBuffer>> ConstantTransformBuffer::vertexBuffer;
}
