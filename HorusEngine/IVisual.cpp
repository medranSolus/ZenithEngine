#include "IVisual.h"

namespace GFX::Visual
{
	void IVisual::Bind(Graphics& gfx) noexcept
	{
		for (auto& bind : binds)
			bind->Bind(gfx);
	}
}