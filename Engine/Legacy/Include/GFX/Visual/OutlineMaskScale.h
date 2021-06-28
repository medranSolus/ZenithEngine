#pragma once
#include "IVisual.h"
#include "GFX/Resource/ConstBufferExCache.h"

namespace ZE::GFX::Visual
{
	class OutlineMaskScale : public IVisual
	{
		GfxResPtr<Resource::ConstBufferExPixelCache> pixelBuffer;
		Data::CBuffer::DynamicCBuffer buffer;
		mutable bool dirty = false;

		static Data::CBuffer::DCBLayout MakeLayout() noexcept;

		void UpdateTransform() const noexcept;

	public:
		OutlineMaskScale(Graphics& gfx, const std::string& tag, const ColorF3& color,
			const std::shared_ptr<Data::VertexLayout>& vertexLayout);
		OutlineMaskScale(OutlineMaskScale&&) = default;
		OutlineMaskScale(const OutlineMaskScale&) = default;
		OutlineMaskScale& operator=(OutlineMaskScale&&) = default;
		OutlineMaskScale& operator=(const OutlineMaskScale&) = default;
		virtual ~OutlineMaskScale() = default;

		bool Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override { return probe.VisitObject(buffer) || pixelBuffer->Accept(gfx, probe); }

		void SetTransformBuffer(Graphics& gfx, const GfxObject& parent) const override;
		void Bind(Graphics& gfx) const override;
	};
}