#pragma once
#include "BaseShape.h"
#include "Object.h"

namespace GFX::Shape
{
	class Triangle : public BaseShape, public Object
	{
	public:
		Triangle(Graphics& gfx, Pipeline::RenderGraph& graph, const DirectX::XMFLOAT3& position,
			const std::string& name, float down, float left, float right);
		virtual ~Triangle() = default;

		void Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override;
	};
}