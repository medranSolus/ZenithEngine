#pragma once
#include "IProbeable.h"

namespace GFX
{
	class IRenderable : public Probe::IProbeable
	{
	public:
		virtual ~IRenderable() = default;

		virtual void SetOutline() noexcept = 0;
		virtual void DisableOutline() noexcept = 0;
		virtual void Submit(uint64_t channelFilter) noexcept = 0;
	};
}