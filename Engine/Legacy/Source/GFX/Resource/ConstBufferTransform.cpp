#include "GFX/Resource/ConstBufferTransform.h"

namespace ZE::GFX::Resource
{
	Data::CBuffer::Transform ConstBufferTransform::GetBufferData(Graphics& gfx) const noexcept
	{
		const Matrix transform = GetTransform();
		return
		{
			Math::XMMatrixTranspose(transform),
			Math::XMMatrixTranspose(transform * gfx.GetView() * gfx.GetProjection())
		};
	}

	void ConstBufferTransform::UpdateBind(Graphics& gfx, const Data::CBuffer::Transform& buffer) const
	{
		vertexBuffer->Update(gfx, buffer);
		vertexBuffer->Bind(gfx);
	}

	ConstBufferTransform::ConstBufferTransform(Graphics& gfx, const GfxObject& parent, U32 slot) : parent(parent)
	{
		if (!vertexBuffer)
			vertexBuffer = std::make_unique<ConstBufferVertex<Data::CBuffer::Transform>>(gfx, "", slot);
	}

	Float3 ConstBufferTransform::GetPos() const noexcept
	{
		const Float4x4& transform = parent.GetTransform();
		return { transform._13, transform._23, transform._33 };
	}
}