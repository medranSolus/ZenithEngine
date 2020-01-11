#include "BaseShape.h"

namespace GFX::Shape
{
	void BaseShape::AddBind(std::shared_ptr<Resource::IBindable> bind) noexcept(!IS_DEBUG)
	{
		if (typeid(*bind) == typeid(Resource::IndexBuffer))
		{
			assert("Attempting to add index buffer a second time" && indexBuffer == nullptr);
			indexBuffer = dynamic_cast<Resource::IndexBuffer*>(bind.get());
		}
		binds.emplace_back(bind);
	}

	void BaseShape::Draw(Graphics & gfx) const noexcept
	{
		for (auto & b : binds)
			b->Bind(gfx);
		gfx.DrawIndexed(indexBuffer->GetCount());
	}
}
