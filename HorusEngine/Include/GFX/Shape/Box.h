#pragma once
#include "IShape.h"

namespace GFX::Shape
{
	class Box : public IShape
	{
		Float3 sizes;

	public:
		Box(Graphics& gfx, Pipeline::RenderGraph& graph, const Float3& position, std::string&& name,
			const ColorF4& color, float width = 1.0f, float height = 1.0f, float length = 1.0f);
		virtual ~Box() = default;

		void SetTopologyMesh(Graphics& gfx) noexcept override { SetTopology(gfx, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ); }

		void UpdateTransformMatrix() noexcept override;
	};
}