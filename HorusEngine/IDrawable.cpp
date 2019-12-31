#include "IDrawable.h"

namespace GFX::Object
{
	void IDrawable::Draw(Graphics & gfx) const noexcept
	{
		for (auto & b : GetStaticBinds())
			b->Bind(gfx);
		for (auto & b : GetBinds())
			b->Bind(gfx);
		gfx.DrawIndexed((GetIndexBuffer() ? GetIndexBuffer() : GetStaticIndexBuffer())->GetCount());
	}
}
