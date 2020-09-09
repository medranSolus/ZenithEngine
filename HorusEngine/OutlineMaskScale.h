#pragma once
#include "Effect.h"

namespace GFX::Visual
{
	class OutlineMaskScale : public Effect
	{
		std::shared_ptr<Resource::ConstBufferExPixelCache> pixelBuffer = nullptr;
		Data::CBuffer::DynamicCBuffer buffer;
		bool dirty = false;

		static Data::CBuffer::DCBLayout MakeLayout() noexcept;

		void UpdateTransform() noexcept;

	public:
		OutlineMaskScale(Graphics& gfx, const std::string& tag, Data::ColorFloat3 color, std::shared_ptr<Data::VertexLayout> vertexLayout);
		virtual ~OutlineMaskScale() = default;

		void SetTransformBuffer(Graphics& gfx, const GfxObject& parent) override;
		void Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override;
		void Bind(Graphics& gfx) override;
	};
}