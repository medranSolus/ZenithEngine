#pragma once
#include "Effect.h"

namespace GFX::Visual
{
	class DepthWrite : public IVisual
	{
	public:
		DepthWrite(Graphics& gfx, std::shared_ptr<Data::VertexLayout> vertexLayout);
		virtual ~DepthWrite() = default;

		inline bool Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override { return false; }
	};
}