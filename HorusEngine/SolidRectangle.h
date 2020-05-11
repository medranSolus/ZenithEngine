#pragma once
#include "BaseShape.h"
#include "Object.h"

namespace GFX::Shape
{
	class SolidRectangle : public BaseShape, public Object
	{
		float width;
		float height;

	public:
		SolidRectangle(Graphics& gfx, const DirectX::XMFLOAT3& position, const std::string& name, Data::ColorFloat3 color, float width, float height);
		SolidRectangle(const SolidRectangle&) = delete;
		SolidRectangle& operator=(const SolidRectangle&) = delete;
		virtual ~SolidRectangle() = default;

		void Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override;
		void UpdateTransformMatrix() noexcept override;
	};
}