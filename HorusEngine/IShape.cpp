#include "IShape.h"

namespace GFX::Shape
{
	void IShape::Draw(Graphics & gfx) const noexcept
	{
		for (auto & b : GetStaticBinds())
			b->Bind(gfx);
		for (auto & b : GetBinds())
			b->Bind(gfx);
		gfx.DrawIndexed((GetIndexBuffer() ? GetIndexBuffer() : GetStaticIndexBuffer())->GetCount());
	}
}
