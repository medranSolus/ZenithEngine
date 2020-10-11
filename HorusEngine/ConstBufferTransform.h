#pragma once
#include "ConstBufferVertex.h"
#include "CBuffers.h"
#include "GfxObject.h"

namespace GFX::Resource
{
	class ConstBufferTransform : public IBindable
	{
		static std::unique_ptr<ConstBufferVertex<Data::CBuffer::Transform>> vertexBuffer;
		const GfxObject* parent;

	protected:
		virtual inline DirectX::FXMMATRIX GetTransform() noexcept { return parent->GetTransformMatrix(); }

		Data::CBuffer::Transform GetBufferData(Graphics& gfx) noexcept;
		virtual void UpdateBind(Graphics& gfx, const Data::CBuffer::Transform& buffer);

	public:
		ConstBufferTransform(Graphics& gfx, const GfxObject& parent, UINT slot = 0U);
		virtual ~ConstBufferTransform() = default;

		inline void ChangeOwner(const GfxObject& owner) noexcept { parent = &owner; }
		inline void Bind(Graphics& gfx) override { UpdateBind(gfx, GetBufferData(gfx)); }
		inline std::string GetRID() const noexcept override { return "?"; }
	};
}