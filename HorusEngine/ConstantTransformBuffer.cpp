#include "ConstantTransformBuffer.h"

namespace GFX::Resource
{
	ConstantTransformBuffer::ConstantTransformBuffer(Graphics & gfx, const Object::IDrawable & parent) : parent(parent)
	{
		if (!vertexBuffer)
			vertexBuffer = std::make_unique<ConstantVertexBuffer<Buffer>>(gfx);
	}

	void ConstantTransformBuffer::Bind(Graphics & gfx) noexcept
	{
		const auto model = parent.GetTransformMatrix();
		Buffer buffer
		{
			model,
			buffer.modelViewProjection = std::move(model * gfx.GetCamera() * gfx.GetProjection())
		};
		vertexBuffer->Update(gfx, buffer);
		vertexBuffer->Bind(gfx);
	}

	std::unique_ptr<ConstantVertexBuffer<ConstantTransformBuffer::Buffer>> ConstantTransformBuffer::vertexBuffer;
}
