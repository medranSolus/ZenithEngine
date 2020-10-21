#pragma once
#include "IShape.h"

namespace GFX::Shape
{
	class SolidCone : public IShape
	{
		float height;
		float angle;
		Resource::ConstBufferExPixelCache* materialBuffer = nullptr;

	public:
		SolidCone(Graphics& gfx, Pipeline::RenderGraph& graph, const DirectX::XMFLOAT3& position, const std::string& name,
			Data::ColorFloat3 color, unsigned int density, float height = 1.0f, float angle = 30.0f);
		virtual ~SolidCone() = default;

		constexpr Resource::ConstBufferExPixelCache& GetMaterial() noexcept { return *materialBuffer; }
		inline void SetTopologyMesh(Graphics& gfx) noexcept override { SetTopology(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ); }

		void UpdateTransformMatrix() noexcept override;
	};
}