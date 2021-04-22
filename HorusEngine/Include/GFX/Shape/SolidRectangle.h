#pragma once
#include "IShape.h"

namespace GFX::Shape
{
	class SolidRectangle : public IShape
	{
		float width;
		float height;

	public:
		SolidRectangle(Graphics& gfx, Pipeline::RenderGraph& graph, const Float3& position,
			std::string&& name, const ColorF3& color, float width, float height);
		virtual ~SolidRectangle() = default;

		void UpdateTransformMatrix() noexcept override;
	};
}