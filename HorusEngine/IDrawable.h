#pragma once
#include "Graphics.h"

namespace GFX
{
	class IDrawable
	{
	public:
		virtual ~IDrawable() = default;

		virtual void Draw(Graphics& gfx) const noexcept = 0;
	};
}