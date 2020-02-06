#include "ConstantTransformBufferVS.h"

namespace GFX::Resource
{
	TransformConstatBuffer ConstantTransformBufferVS::GetBufferData(Graphics& gfx) noexcept
	{
		const DirectX::XMMATRIX transformView = std::move(parent.GetTransformMatrix() * gfx.GetCamera());
		return
		{
				std::move(DirectX::XMMatrixTranspose(transformView)),
				std::move(DirectX::XMMatrixTranspose(transformView * gfx.GetProjection()))
		};
	}

	void ConstantTransformBufferVS::UpdateBind(Graphics& gfx, const TransformConstatBuffer& buffer) noexcept
	{
		vertexBuffer->Update(gfx, buffer);
		vertexBuffer->Bind(gfx);
	}

	ConstantTransformBufferVS::ConstantTransformBufferVS(Graphics& gfx, const GfxObject& parent, UINT slot) : parent(parent)
	{
		if (!vertexBuffer)
			vertexBuffer = std::make_unique<ConstantVertexBuffer<TransformConstatBuffer>>(gfx, "", slot);
	}

	std::unique_ptr<ConstantVertexBuffer<TransformConstatBuffer>> ConstantTransformBufferVS::vertexBuffer;
}