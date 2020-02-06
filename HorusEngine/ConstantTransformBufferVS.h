#pragma once
#include "ConstantVertexBuffer.h"
#include "ShaderConstantBuffers.h"
#include "GfxObject.h"

namespace GFX::Resource
{
	class ConstantTransformBufferVS : public IBindable
	{
		static std::unique_ptr<ConstantVertexBuffer<TransformConstatBuffer>> vertexBuffer;
		const GfxObject& parent;

	protected:
		TransformConstatBuffer GetBufferData(Graphics& gfx) noexcept;
		virtual void UpdateBind(Graphics& gfx, const TransformConstatBuffer& buffer) noexcept;

	public:
		ConstantTransformBufferVS(Graphics& gfx, const GfxObject& parent, UINT slot = 0U);
		ConstantTransformBufferVS(const ConstantTransformBufferVS&) = delete;
		ConstantTransformBufferVS& operator=(const ConstantTransformBufferVS&) = delete;
		virtual ~ConstantTransformBufferVS() = default;

		inline void Bind(Graphics& gfx) noexcept override { UpdateBind(gfx, GetBufferData(gfx)); }
		inline std::string GetRID() const noexcept override { return "?"; }
	};
}