#pragma once
#include "IVisual.h"
#include "ConstBufferExCache.h"

namespace GFX::Visual
{
	class OutlineMaskScale : public IVisual
	{
		GfxResPtr<Resource::ConstBufferExPixelCache> pixelBuffer;
		Data::CBuffer::DynamicCBuffer buffer;
		bool dirty = false;

		static inline Data::CBuffer::DCBLayout MakeLayout() noexcept;

		void UpdateTransform() noexcept;

	public:
		OutlineMaskScale(Graphics& gfx, const std::string& tag, Data::ColorFloat3 color, std::shared_ptr<Data::VertexLayout> vertexLayout);
		virtual ~OutlineMaskScale() = default;

		void SetTransformBuffer(Graphics& gfx, const GfxObject& parent) override;
		bool Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override;
		void Bind(Graphics& gfx) override;
	};
}