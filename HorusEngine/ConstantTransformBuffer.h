#pragma once
#include "ConstantVertexBuffer.h"
#include "ShaderConstantBuffers.h"
#include "GfxObject.h"

namespace GFX::Resource
{
	class ConstantTransformBuffer : public IBindable
	{
		static std::unique_ptr<ConstantVertexBuffer<TransformConstatBuffer>> vertexBuffer;
		const GfxObject & parent;

	public:
		ConstantTransformBuffer(Graphics & gfx, const GfxObject & parent, UINT slot = 0U);
		ConstantTransformBuffer(const ConstantTransformBuffer&) = delete;
		ConstantTransformBuffer & operator=(const ConstantTransformBuffer&) = delete;
		~ConstantTransformBuffer() = default;

		void Bind(Graphics & gfx) noexcept override;
		inline std::string GetRID() const noexcept override { return "?"; }
	};
}
