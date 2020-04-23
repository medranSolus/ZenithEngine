#include "ConstBufferTransform.h"
namespace GFX::Resource
{
	Data::CBuffer::Transform ConstBufferTransform::GetBufferData(Graphics& gfx) noexcept
	{
		const DirectX::XMMATRIX transformView = std::move(parent.GetTransformMatrix() * gfx.GetCamera());
		return
		{
			std::move(DirectX::XMMatrixTranspose(transformView)),
			std::move(DirectX::XMMatrixTranspose(transformView * gfx.GetProjection()))
		};
	}

	void ConstBufferTransform::UpdateBind(Graphics& gfx, const Data::CBuffer::Transform& buffer) noexcept
	{
		vertexBuffer->Update(gfx, buffer);
		vertexBuffer->Bind(gfx);
	}

	ConstBufferTransform::ConstBufferTransform(Graphics& gfx, const GfxObject& parent, UINT slot) : parent(parent)
	{
		if (!vertexBuffer)
			vertexBuffer = std::make_unique<ConstBufferVertex<Data::CBuffer::Transform>>(gfx, "", slot);
	}

	std::unique_ptr<ConstBufferVertex<Data::CBuffer::Transform>> ConstBufferTransform::vertexBuffer;
}