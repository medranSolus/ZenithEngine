#pragma once
#include "BaseShape.h"
#include "Object.h"

namespace GFX::Shape
{
	class Globe : public BaseShape, public Object
	{
		DirectX::XMFLOAT3 sizes;

	public:
		Globe(Graphics& gfx, const DirectX::XMFLOAT3& position, const std::string& name, Data::ColorFloat4 material,
			unsigned int latitudeDensity, unsigned int longitudeDensity, float width = 1.0f, float height = 1.0f, float length = 1.0f);
		Globe(const Globe&) = delete;
		Globe& operator=(const Globe&) = delete;
		virtual ~Globe() = default;

		inline void SetTopologyMesh(Graphics& gfx) noexcept override { SetTopology(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ); }

		void Accept(Probe& probe) noexcept override;
		void UpdateTransformMatrix() noexcept override;
	};
}