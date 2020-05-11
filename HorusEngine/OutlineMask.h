#pragma once
#include "Effect.h"

namespace GFX::Visual
{
	class OutlineMask : public Effect
	{
	public:
		OutlineMask(Graphics& gfx, Data::ColorFloat3 color, std::shared_ptr<Data::VertexLayout> vertexLayout);
		virtual ~OutlineMask() = default;

		inline void Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override {}
	};
}