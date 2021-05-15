#pragma once
#include "IShape.h"

namespace ZE::GFX::Shape
{
	class SolidGlobe : public IShape
	{
		Float3 sizes;
		Resource::ConstBufferExPixelCache* materialBuffer = nullptr;

	public:
		SolidGlobe(Graphics& gfx, Pipeline::RenderGraph& graph, const Float3& position, std::string&& name, const ColorF3& color,
			U32 latitudeDensity, U32 longitudeDensity, float width = 1.0f, float height = 1.0f, float length = 1.0f);
		virtual ~SolidGlobe() = default;

		constexpr Resource::ConstBufferExPixelCache& GetMaterial() noexcept { return *materialBuffer; }
		void SetTopologyMesh(Graphics& gfx) noexcept override { SetTopology(gfx, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ); }

		void UpdateTransformMatrix() noexcept override;
	};
}