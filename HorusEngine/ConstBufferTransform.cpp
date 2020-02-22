#include "ConstBufferTransform.h"
namespace GFX::Resource
{
	TransformCBuffer ConstBufferTransform::GetBufferData(Graphics& gfx) noexcept
	{
		const DirectX::XMMATRIX transformView = std::move(parent.GetTransformMatrix() * gfx.GetCamera());
		return
		{
				std::move(DirectX::XMMatrixTranspose(transformView)),
				std::move(DirectX::XMMatrixTranspose(transformView * gfx.GetProjection()))
		};
	}

	void ConstBufferTransform::UpdateBind(Graphics& gfx, const TransformCBuffer& buffer) noexcept
	{
		vertexBuffer->Update(gfx, buffer);
		vertexBuffer->Bind(gfx);
	}

	ConstBufferTransform::ConstBufferTransform(Graphics& gfx, const GfxObject& parent, UINT slot) : parent(parent)
	{
		if (!vertexBuffer)
			vertexBuffer = std::make_unique<ConstBufferVertex<TransformCBuffer>>(gfx, "", slot);
	}

	std::unique_ptr<ConstBufferVertex<TransformCBuffer>> ConstBufferTransform::vertexBuffer;
}