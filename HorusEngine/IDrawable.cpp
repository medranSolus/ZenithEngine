#include "IDrawable.h"

namespace GFX::Object
{
	void IDrawable::AddBind(std::unique_ptr<Resource::IBindable> bind) noexcept(!IS_DEBUG)
	{
		assert("*Must* use AddIndexBuffer to bind index buffer" && typeid(*bind) != typeid(Resource::IndexBuffer));
		binds.push_back(std::move(bind));
	}

	void IDrawable::Draw(Graphics & gfx) const noexcept
	{
		for (auto & b : GetStaticBinds())
			b->Bind(gfx);
		for (auto & b : binds)
			b->Bind(gfx);
		gfx.DrawIndexed(GetStaticIndexBuffer()->GetCount());
	}
}
