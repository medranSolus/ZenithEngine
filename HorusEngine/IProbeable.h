#pragma once
#include "ModelProbe.h"

namespace GFX::Probe
{
	class IProbeable
	{
	public:
		virtual ~IProbeable() = default;

		virtual void Accept(Graphics& gfx, BaseProbe& probe) noexcept = 0;
		virtual inline void Accept(Graphics& gfx, ModelProbe& probe) noexcept { Accept(gfx, static_cast<BaseProbe&>(probe)); }
	};
}