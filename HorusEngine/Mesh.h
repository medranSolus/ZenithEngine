#pragma once
#include "BaseShape.h"
#include "GfxObject.h"

namespace GFX::Shape
{
	class Mesh : public BaseShape, public GfxObject
	{
	public:
		Mesh(Graphics& gfx, std::vector<std::shared_ptr<Resource::IBindable>>&& binds);
		Mesh(const Mesh&) = delete;
		Mesh& operator=(const Mesh&) = delete;
		virtual ~Mesh() = default;
	};
}
