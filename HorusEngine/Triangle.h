#pragma once
#include "BaseShape.h"
#include "Object.h"

namespace GFX::Shape
{
	class Triangle : public BaseShape<Triangle>, public Object
	{
	public:
		Triangle(Graphics & gfx, const DirectX::XMFLOAT3 & position, const std::string & name, float down, float left, float right);
		
		DirectX::XMMATRIX GetTransformMatrix() const noexcept override;
	};
}