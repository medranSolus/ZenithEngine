#include "IVisual.h"

namespace GFX::Visual
{
	void IVisual::Bind(Graphics& gfx) noexcept
	{
		transformBuffer->Bind(gfx);
		for (auto& bind : binds)
			bind->Bind(gfx);
	}
}