#pragma once
#include "ConstBuffer.h"
#include "GFX/Data/CBuffer/CBuffers.h"
#include "GFX/GfxObject.h"

namespace ZE::GFX::Resource
{
	class ConstBufferTransform : public IBindable
	{
		static inline std::unique_ptr<ConstBufferVertex<Data::CBuffer::Transform>> vertexBuffer;

		const GfxObject& parent;

	protected:
		Data::CBuffer::Transform GetBufferData(Graphics& gfx) const noexcept;
		virtual void UpdateBind(Graphics& gfx, const Data::CBuffer::Transform& buffer) const;

	public:
		ConstBufferTransform(Graphics& gfx, const GfxObject& parent, U32 slot = 0);
		virtual ~ConstBufferTransform() = default;

		virtual Matrix GetTransform() const noexcept { return parent.GetTransformMatrix(); }
		void Bind(Graphics& gfx) const override { UpdateBind(gfx, GetBufferData(gfx)); }

		virtual Float3 GetPos() const noexcept;
	};
}