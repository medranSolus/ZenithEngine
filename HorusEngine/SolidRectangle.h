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
		SolidRectangle(Graphics& gfx, Pipeline::RenderGraph& graph, const DirectX::XMFLOAT3& position,
			const std::string& name, Data::ColorFloat3 color, float width, float height);
		virtual ~SolidRectangle() = default;

		void Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override;
		void UpdateTransformMatrix() noexcept override;
	};
}