#pragma once
#include "IShape.h"

namespace GFX::Shape
{
	class Ball : public IShape
	{
	public:
		Ball(Graphics& gfx, Pipeline::RenderGraph& graph, const DirectX::XMFLOAT3& position,
			const std::string& name, Data::ColorFloat4 color, unsigned int density, float radius);
		virtual ~Ball() = default;

		inline void SetTopologyMesh(Graphics& gfx) noexcept override { SetTopology(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ); }
	};
}