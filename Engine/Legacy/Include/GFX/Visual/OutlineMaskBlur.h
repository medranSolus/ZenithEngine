#pragma once
#include "IVisual.h"
#include "GFX/Resource/ConstBufferExCache.h"

namespace ZE::GFX::Visual
{
	class OutlineMaskBlur : public IVisual
	{
		GfxResPtr<Resource::ConstBufferExPixelCache> pixelBuffer;

	public:
		OutlineMaskBlur(Graphics& gfx, const std::string& tag, const ColorF3& color,
			const std::shared_ptr<Data::VertexLayout>& vertexLayout);
		OutlineMaskBlur(OutlineMaskBlur&&) = default;
		OutlineMaskBlur(const OutlineMaskBlur&) = default;
		OutlineMaskBlur& operator=(OutlineMaskBlur&&) = default;
		OutlineMaskBlur& operator=(const OutlineMaskBlur&) = default;
		virtual ~OutlineMaskBlur() = default;

		bool Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override { return pixelBuffer->Accept(gfx, probe); }

		void Bind(Graphics& gfx) const override;
	};
}