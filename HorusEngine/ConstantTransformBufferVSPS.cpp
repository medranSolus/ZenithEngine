#include "ConstantTransformBufferVSPS.h"

namespace GFX::Resource
{
	void ConstantTransformBufferVSPS::UpdateBind(Graphics& gfx, const TransformConstatBuffer& buffer) noexcept
	{
		pixelBuffer->Update(gfx, buffer);
		pixelBuffer->Bind(gfx);
	}

	ConstantTransformBufferVSPS::ConstantTransformBufferVSPS(Graphics& gfx, const GfxObject& parent, UINT slotVS, UINT slotPS)
		: ConstantTransformBufferVS(gfx, parent, slotVS)
	{
		if (!pixelBuffer)
			pixelBuffer = std::make_unique<ConstantPixelBuffer<TransformConstatBuffer>>(gfx, "", slotPS);
	}

	void ConstantTransformBufferVSPS::Bind(Graphics& gfx) noexcept
	{
		const auto buffer = GetBufferData(gfx);
		ConstantTransformBufferVS::UpdateBind(gfx, buffer);
		UpdateBind(gfx, buffer);
	}

	std::unique_ptr<ConstantPixelBuffer<TransformConstatBuffer>> ConstantTransformBufferVSPS::pixelBuffer;
}