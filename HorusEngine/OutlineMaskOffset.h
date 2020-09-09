#pragma once
#include "Effect.h"

namespace GFX::Visual
{
	class OutlineMaskOffset : public Effect
	{
		std::shared_ptr<Resource::ConstBufferExVertexCache> vertexBuffer = nullptr;
		std::shared_ptr<Resource::ConstBufferExPixelCache> pixelBuffer = nullptr;

	public:
		OutlineMaskOffset(Graphics& gfx, const std::string& tag, Data::ColorFloat3 color, std::shared_ptr<Data::VertexLayout> vertexLayout);
		virtual ~OutlineMaskOffset() = default;

		void Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override;
		void Bind(Graphics& gfx) override;
	};
}