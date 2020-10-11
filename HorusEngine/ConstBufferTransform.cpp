#include "ConstBufferTransform.h"

namespace GFX::Resource
{
	std::unique_ptr<ConstBufferVertex<Data::CBuffer::Transform>> ConstBufferTransform::vertexBuffer;

	Data::CBuffer::Transform ConstBufferTransform::GetBufferData(Graphics& gfx) noexcept
	{
		const DirectX::XMMATRIX transform = GetTransform();
		return
		{
			std::move(DirectX::XMMatrixTranspose(transform)),
			std::move(DirectX::XMMatrixTranspose(transform * gfx.GetView() * gfx.GetProjection()))
		};
	}

	void ConstBufferTransform::UpdateBind(Graphics& gfx, const Data::CBuffer::Transform& buffer)
	{
		vertexBuffer->Update(gfx, buffer);
		vertexBuffer->Bind(gfx);
	}

	ConstBufferTransform::ConstBufferTransform(Graphics& gfx, const GfxObject& parent, UINT slot) : parent(&parent)
	{
		if (!vertexBuffer)
			vertexBuffer = std::make_unique<ConstBufferVertex<Data::CBuffer::Transform>>(gfx, "", slot);
	}
}