#pragma once
#include "BaseShape.h"
#include "Object.h"

namespace GFX::Shape
{
	class IShape : public BaseShape, public Object
	{
	public:
		inline IShape(Graphics& gfx, const DirectX::XMFLOAT3& position, const std::string& name, float scale = 1.0f)
			: BaseShape(gfx), Object(position, name, scale) {}
		virtual ~IShape() = default;

		inline const std::string& GetName() const noexcept { return name; }
		inline bool Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override { return Object::Accept(gfx, probe) || BaseShape::Accept(gfx, probe); }
	};
}