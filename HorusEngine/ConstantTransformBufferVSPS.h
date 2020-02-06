#pragma once
#include "ConstantTransformBufferVS.h"
#include "ConstantPixelBuffer.h"

namespace GFX::Resource
{
	class ConstantTransformBufferVSPS : public ConstantTransformBufferVS
	{
		static std::unique_ptr<ConstantPixelBuffer<TransformConstatBuffer>> pixelBuffer;

	protected:
		void UpdateBind(Graphics& gfx, const TransformConstatBuffer& buffer) noexcept override;

	public:
		ConstantTransformBufferVSPS(Graphics& gfx, const GfxObject& parent, UINT slotVS = 0U, UINT slotPS = 0U);
		ConstantTransformBufferVSPS(const ConstantTransformBufferVSPS&) = delete;
		ConstantTransformBufferVSPS& operator=(const ConstantTransformBufferVSPS&) = delete;
		virtual ~ConstantTransformBufferVSPS() = default;

		void Bind(Graphics& gfx) noexcept override;
	};
}