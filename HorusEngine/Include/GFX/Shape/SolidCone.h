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
		SolidCone(Graphics& gfx, Pipeline::RenderGraph& graph, const Float3& position, std::string&& name,
			const ColorF3& color, U32 density, float height = 1.0f, float angle = 30.0f);
		virtual ~SolidCone() = default;

		constexpr Resource::ConstBufferExPixelCache& GetMaterial() noexcept { return *materialBuffer; }
		void SetTopologyMesh(Graphics& gfx) noexcept override { SetTopology(gfx, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ); }

		void UpdateTransformMatrix() noexcept override;
	};
}