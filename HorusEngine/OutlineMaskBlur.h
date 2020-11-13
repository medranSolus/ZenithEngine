#pragma once
#include "IVisual.h"
#include "ConstBufferExCache.h"

namespace GFX::Visual
{
	class OutlineMaskBlur : public IVisual
	{
		GfxResPtr<Resource::ConstBufferExPixelCache> pixelBuffer;

	public:
		OutlineMaskBlur(Graphics& gfx, const std::string& tag, Data::ColorFloat3 color, std::shared_ptr<Data::VertexLayout> vertexLayout);
		virtual ~OutlineMaskBlur() = default;

		inline bool Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override { return pixelBuffer->Accept(gfx, probe); }

		void Bind(Graphics& gfx) override;
	};
}