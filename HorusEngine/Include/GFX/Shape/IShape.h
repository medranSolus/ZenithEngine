#pragma once
#include "BaseShape.h"
#include "GFX/Object.h"

namespace GFX::Shape
{
	class IShape : public BaseShape, public Object
	{
	public:
		IShape(Graphics& gfx, const Float3& position, std::string&& name, float scale = 1.0f)
			: BaseShape(gfx), Object(position, std::forward<std::string>(name), scale) {}
		virtual ~IShape() = default;

		constexpr const std::string& GetName() const noexcept { return name; }
		bool Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override { return Object::Accept(gfx, probe) || BaseShape::Accept(gfx, probe); }
	};
}