#pragma once
#include "BaseShape.h"
#include "Object.h"

namespace GFX::Shape
{
	class Rectangle : public BaseShape, public Object
	{
		float width;
		float height;

	public:
		Rectangle(Graphics& gfx, const DirectX::XMFLOAT3& position, const std::string& name, float width, float height);
		Rectangle(const Rectangle&) = delete;
		Rectangle& operator=(const Rectangle&) = delete;
		virtual ~Rectangle() = default;

		void Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override;
		void UpdateTransformMatrix() noexcept override;
	};
}