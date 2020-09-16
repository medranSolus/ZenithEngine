#pragma once
#include "IShape.h"

namespace GFX::Shape
{
	class SolidRectangle : public IShape
	{
		float width;
		float height;

	public:
		SolidRectangle(Graphics& gfx, Pipeline::RenderGraph& graph, const DirectX::XMFLOAT3& position,
			const std::string& name, Data::ColorFloat3 color, float width, float height);
		virtual ~SolidRectangle() = default;

		void UpdateTransformMatrix() noexcept override;
	};
}