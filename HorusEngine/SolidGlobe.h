#pragma once
#include "BaseShape.h"
#include "Object.h"

namespace GFX::Shape
{
	class SolidGlobe : public BaseShape, public Object
	{
		DirectX::XMFLOAT3 sizes;

	public:
		SolidGlobe(Graphics& gfx, const DirectX::XMFLOAT3& position, const std::string& name, Data::ColorFloat4 material,
			unsigned int latitudeDensity, unsigned int longitudeDensity, float width = 1.0f, float height = 1.0f, float length = 1.0f);
		SolidGlobe(const SolidGlobe&) = delete;
		SolidGlobe& operator=(const SolidGlobe&) = delete;
		virtual ~SolidGlobe() = default;

		void UpdateTransformMatrix() noexcept override;
	};
}
