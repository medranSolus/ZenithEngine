#pragma once
#include "IShape.h"

namespace GFX::Shape
{
	class Ball : public IShape
	{
	public:
		Ball(Graphics& gfx, Pipeline::RenderGraph& graph, const Float3& position,
			std::string&& name, const ColorF4& color, U32 density, float radius);
		virtual ~Ball() = default;

		void SetTopologyMesh(Graphics& gfx) noexcept override { SetTopology(gfx, D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ); }
	};
}