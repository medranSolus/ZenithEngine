#pragma once
#include "Effect.h"

namespace GFX::Visual
{
	class OutlineMask : public Effect
	{
		std::shared_ptr<Resource::ConstBufferExPixelCache> pixelBuffer = nullptr;
		Data::CBuffer::DynamicCBuffer buffer;
		bool dirty = false;

		static Data::CBuffer::DCBLayout MakeLayout() noexcept;

		void UpdateTransform() noexcept;

	public:
		OutlineMask(Graphics& gfx, const std::string& tag, Data::ColorFloat3 color, std::shared_ptr<Data::VertexLayout> vertexLayout);
		virtual ~OutlineMask() = default;

		void SetTransformBuffer(Graphics& gfx, const GfxObject& parent) override;
		void Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override;
		void Bind(Graphics& gfx) noexcept override;
	};
}