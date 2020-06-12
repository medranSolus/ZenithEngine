#pragma once
#include "Effect.h"

namespace GFX::Visual
{
	class OutlineMaskBlur : public Effect
	{
		std::shared_ptr<Resource::ConstBufferExPixelCache> pixelBuffer = nullptr;

	public:
		OutlineMaskBlur(Graphics& gfx, const std::string& tag, Data::ColorFloat3 color, std::shared_ptr<Data::VertexLayout> vertexLayout);
		virtual ~OutlineMaskBlur() = default;

		void Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override;
		void Bind(Graphics& gfx) noexcept override;
	};
}