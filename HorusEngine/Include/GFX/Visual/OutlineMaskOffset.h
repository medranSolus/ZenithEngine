#pragma once
#include "IVisual.h"
#include "GFX/Resource/ConstBufferExCache.h"

namespace GFX::Visual
{
	class OutlineMaskOffset : public IVisual
	{
		GfxResPtr<Resource::ConstBufferExVertexCache> vertexBuffer;
		GfxResPtr<Resource::ConstBufferExPixelCache> pixelBuffer;

	public:
		OutlineMaskOffset(Graphics& gfx, const std::string& tag, const ColorF3& color,
			const std::shared_ptr<Data::VertexLayout>& vertexLayout);
		OutlineMaskOffset(OutlineMaskOffset&&) = default;
		OutlineMaskOffset(const OutlineMaskOffset&) = default;
		OutlineMaskOffset& operator=(OutlineMaskOffset&&) = default;
		OutlineMaskOffset& operator=(const OutlineMaskOffset&) = default;
		virtual ~OutlineMaskOffset() = default;

		bool Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override { return vertexBuffer->Accept(gfx, probe) || pixelBuffer->Accept(gfx, probe); }

		void Bind(Graphics& gfx) const override;
	};
}