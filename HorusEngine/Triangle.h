#pragma once
#include "IShape.h"

namespace GFX::Shape
{
	class Triangle : public IShape
	{
	public:
		Triangle(Graphics& gfx, Pipeline::RenderGraph& graph, const DirectX::XMFLOAT3& position,
			const std::string& name, float down, float left, float right);
		virtual ~Triangle() = default;
	};
}