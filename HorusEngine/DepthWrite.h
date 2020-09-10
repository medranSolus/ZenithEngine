#pragma once
#include "Material.h"

namespace GFX::Visual
{
	class DepthWrite : public IVisual
	{
	public:
		DepthWrite(Graphics& gfx, std::shared_ptr<Material> material);
		virtual ~DepthWrite() = default;

		inline void Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override {}
	};
}