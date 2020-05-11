#pragma once
#include "Effect.h"

namespace GFX::Visual
{
	class OutlineWrite : public Effect
	{
	public:
		OutlineWrite(Graphics& gfx, std::shared_ptr<Data::VertexLayout> vertexLayout);
		virtual ~OutlineWrite() = default;

		inline void Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override {}
	};
}