#include "Mesh.h"
#include "Primitives.h"
#include "GfxResources.h"
#include "Math.h"

namespace GFX::Shape
{
	Mesh::Mesh(Graphics& gfx, std::vector<std::shared_ptr<Resource::IBindable>>&& binds)
		: BaseShape(gfx, *this), GfxObject(false)
	{
		for (auto& bind : binds)
			AddBind(bind);
	}
}