#pragma once
#include "IVisual.h"
#include "ConstBufferExCache.h"

namespace GFX::Visual
{
	class OutlineMaskOffset : public IVisual
	{
		GfxResPtr<Resource::ConstBufferExVertexCache> vertexBuffer;
		GfxResPtr<Resource::ConstBufferExPixelCache> pixelBuffer;

	public:
		OutlineMaskOffset(Graphics& gfx, const std::string& tag, Data::ColorFloat3 color, std::shared_ptr<Data::VertexLayout> vertexLayout);
		virtual ~OutlineMaskOffset() = default;

		inline bool Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override { return vertexBuffer->Accept(gfx, probe) || pixelBuffer->Accept(gfx, probe); }

		void Bind(Graphics& gfx) override;
	};
}