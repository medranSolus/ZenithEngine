#include "GFX/Visual/IVisual.h"

namespace GFX::Visual
{
	void IVisual::Bind(Graphics& gfx) const
	{
		transformBuffer->Bind(gfx);
		for (auto& bind : binds)
			bind->Bind(gfx);
	}
}