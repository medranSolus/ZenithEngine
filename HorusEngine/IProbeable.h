#pragma once
#include "Probe.h"

namespace GFX
{
	class IProbeable
	{
	public:
		virtual ~IProbeable() = default;

		virtual void Accept(Probe& probe) noexcept = 0;
	};
}