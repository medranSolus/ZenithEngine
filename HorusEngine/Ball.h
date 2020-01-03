#pragma once
#include "BaseShape.h"
#include "Object.h"

namespace GFX::Shape
{
	class Ball : public BaseShape<Ball>, public Object
	{
		float r;

	public:
		Ball(Graphics & gfx, const DirectX::XMFLOAT3 & position, const std::string & name, BasicType::ColorFloat material, unsigned int density, float radius);

		DirectX::XMMATRIX GetTransformMatrix() const noexcept override;
	};
}
