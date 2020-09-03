#pragma once
#include "IVisual.h"

namespace GFX::Visual
{
	class ShadowMap : public IVisual
	{
	public:
		ShadowMap(Graphics& gfx, std::shared_ptr<Data::VertexLayout> vertexLayout);
		virtual ~ShadowMap() = default;

		inline void Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override {}
	};
}