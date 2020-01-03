#pragma once
#include "BaseShape.h"
#include "Object.h"

namespace GFX::Shape
{
	class Rectangle : public BaseShape<Rectangle>, public Object
	{
		float width;
		float height;

	public:
		Rectangle(Graphics & gfx, const DirectX::XMFLOAT3 & position, const std::string & name, float width, float height, bool isRandom = false);
	
		DirectX::XMMATRIX GetTransformMatrix() const noexcept override;
	};
}
