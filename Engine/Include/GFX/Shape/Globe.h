#pragma once
#include "IShape.h"

namespace ZE::GFX::Shape
{
	class Globe : public IShape
	{
		Float3 sizes;

	public:
		Globe(Graphics& gfx, Pipeline::RenderGraph& graph, const Float3& position, std::string&& name, const ColorF4& color,
			U32 latitudeDensity, U32 longitudeDensity, float width = 1.0f, float height = 1.0f, float length = 1.0f);
		virtual ~Globe() = default;

		void SetTopologyMesh(Graphics& gfx) noexcept override { SetTopology(gfx, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ); }

		void UpdateTransformMatrix() noexcept override;
	};
}