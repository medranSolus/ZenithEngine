#pragma once
#include "Effect.h"
#include "ConstBufferExCache.h"

namespace GFX::Visual
{
	class OutlineMaskBlur : public Effect
	{
		std::shared_ptr<Resource::ConstBufferExPixelCache> pixelBuffer = nullptr;

	public:
		OutlineMaskBlur(Graphics& gfx, const std::string& tag, Data::ColorFloat3 color, std::shared_ptr<Data::VertexLayout> vertexLayout);
		virtual ~OutlineMaskBlur() = default;

		inline void Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override { pixelBuffer->Accept(gfx, probe); }

		void Bind(Graphics& gfx) override;
	};
}