#pragma once
#include "BaseShape.h"
#include "Object.h"

namespace GFX::Shape
{
	class SolidGlobe : public BaseShape<SolidGlobe>, public Object
	{
		DirectX::XMFLOAT3 size;

	public:
		SolidGlobe(Graphics & gfx, const DirectX::XMFLOAT3 & position, const std::string & name, BasicType::ColorFloat material, unsigned int latitudeDensity, unsigned int longitudeDensity,
			float height = 1.0f, float width = 1.0f, float length = 1.0f);
		
		DirectX::XMMATRIX GetTransformMatrix() const noexcept override;
	};
}
