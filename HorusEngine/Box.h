#pragma once
#include "IShape.h"

namespace GFX::Shape
{
	class Box : public IShape
	{
		DirectX::XMFLOAT3 sizes;

	public:
		Box(Graphics& gfx, Pipeline::RenderGraph& graph, const DirectX::XMFLOAT3& position, const std::string& name,
			Data::ColorFloat4 color, float width = 1.0f, float height = 1.0f, float length = 1.0f);
		virtual ~Box() = default;

		inline void SetTopologyMesh(Graphics& gfx) noexcept override { SetTopology(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ); }

		void UpdateTransformMatrix() noexcept override;
	};
}