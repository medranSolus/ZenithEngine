#pragma once
#include "BaseShape.h"
#include "Object.h"

namespace GFX::Shape
{
	class Ball : public BaseShape<Ball>, public Object
	{
	public:
		Ball(Graphics & gfx, const DirectX::XMFLOAT3 & position, const std::string & name, BasicType::ColorFloat material, unsigned int density, float radius);
		Ball(const Ball&) = delete;
		Ball & operator=(const Ball&) = delete;
		virtual ~Ball() = default;
	};
}
