#pragma once
#include "Effect.h"

namespace GFX::Visual
{
	class OutlineDraw : public Effect
	{
	public:
		OutlineDraw(Graphics& gfx, std::shared_ptr<Data::VertexLayout> vertexLayout);
		virtual ~OutlineDraw() = default;

		void Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override {}
	};
}