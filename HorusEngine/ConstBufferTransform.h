#pragma once
#include "ConstBufferVertex.h"
#include "CBuffers.h"
#include "GfxObject.h"

namespace GFX::Resource
{
	class ConstBufferTransform : public IBindable
	{
		static std::unique_ptr<ConstBufferVertex<Data::CBuffer::Transform>> vertexBuffer;
		const GfxObject& parent;

	protected:
		Data::CBuffer::Transform GetBufferData(Graphics& gfx) noexcept;
		virtual void UpdateBind(Graphics& gfx, const Data::CBuffer::Transform& buffer);

	public:
		ConstBufferTransform(Graphics& gfx, const GfxObject& parent, UINT slot = 0U);
		virtual ~ConstBufferTransform() = default;

		virtual inline DirectX::XMMATRIX GetTransform() const noexcept { return parent.GetTransformMatrix(); }

		inline void Bind(Graphics& gfx) override { UpdateBind(gfx, GetBufferData(gfx)); }
		inline std::string GetRID() const noexcept override { return IBindable::GetNoCodexRID(); }

		virtual DirectX::XMFLOAT3 GetPos() const noexcept;
	};
}