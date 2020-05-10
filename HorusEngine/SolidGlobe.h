#pragma once
#include "BaseShape.h"
#include "Object.h"

namespace GFX::Shape
{
	class SolidGlobe : public BaseShape, public Object
	{
		DirectX::XMFLOAT3 sizes;
		Resource::ConstBufferExPixelCache* materialBuffer = nullptr;

	public:
		SolidGlobe(Graphics& gfx, const DirectX::XMFLOAT3& position, const std::string& name, Data::ColorFloat4 color,
			unsigned int latitudeDensity, unsigned int longitudeDensity, float width = 1.0f, float height = 1.0f, float length = 1.0f);
		SolidGlobe(const SolidGlobe&) = delete;
		SolidGlobe& operator=(const SolidGlobe&) = delete;
		virtual ~SolidGlobe() = default;

		inline Resource::ConstBufferExPixelCache& GetMaterial() noexcept { return *materialBuffer; }
		inline void SetTopologyMesh(Graphics& gfx) noexcept override { SetTopology(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ); }

		void Accept(Probe& probe) noexcept override;
		void UpdateTransformMatrix() noexcept override;
	};
}