#pragma once
#include "ModelProbe.h"

namespace GFX::Probe
{
	class IProbeable
	{
	public:
		virtual ~IProbeable() = default;

		virtual bool Accept(Graphics& gfx, BaseProbe& probe) noexcept = 0;
		virtual bool Accept(Graphics& gfx, ModelProbe& probe) noexcept { return Accept(gfx, static_cast<BaseProbe&>(probe)); }
	};
}