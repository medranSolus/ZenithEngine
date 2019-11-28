#include "ConstantTransformBuffer.h"

namespace GFX::Resource
{
	ConstantTransformBuffer::ConstantTransformBuffer(Graphics & gfx, const Object::IDrawable & parent) : parent(parent)
	{
		if (!vertexBuffer)
			vertexBuffer = std::make_unique<ConstantVertexBuffer<DirectX::XMMATRIX>>(gfx);
	}

	void ConstantTransformBuffer::Bind(Graphics & gfx) noexcept
	{
		vertexBuffer->Update(gfx, std::move(DirectX::XMMatrixTranspose(parent.GetTransformMatrix() * gfx.GetCamera() * gfx.GetProjection())));
		vertexBuffer->Bind(gfx);
	}

	std::unique_ptr<ConstantVertexBuffer<DirectX::XMMATRIX>> ConstantTransformBuffer::vertexBuffer;
}
