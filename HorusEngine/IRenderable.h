#pragma once
#include "IProbeable.h"

namespace GFX
{
	class IRenderable : public Probe::IProbeable
	{
	public:
		virtual ~IRenderable() = default;

		virtual void Submit() noexcept = 0;
	};
}