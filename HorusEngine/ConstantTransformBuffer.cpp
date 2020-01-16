#include "ConstantTransformBuffer.h"

namespace GFX::Resource
{
	ConstantTransformBuffer::ConstantTransformBuffer(Graphics & gfx, const GfxObject & parent, UINT slot) : parent(parent)
	{
		if (!vertexBuffer)
			vertexBuffer = std::make_unique<ConstantVertexBuffer<TransformConstatBuffer>>(gfx, "", slot);
	}

	void ConstantTransformBuffer::Bind(Graphics & gfx) noexcept
	{
		vertexBuffer->Update(gfx,
			{
				std::move(DirectX::XMMatrixTranspose(parent.GetTransformMatrix())),
				std::move(DirectX::XMMatrixTranspose(parent.GetScalingMatrix())),
				std::move(DirectX::XMMatrixTranspose(gfx.GetCamera())),
				std::move(DirectX::XMMatrixTranspose(gfx.GetProjection()))
			});
		vertexBuffer->Bind(gfx);
	}

	std::unique_ptr<ConstantVertexBuffer<TransformConstatBuffer>> ConstantTransformBuffer::vertexBuffer;
}
