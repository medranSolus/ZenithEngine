#pragma once
#include "BaseShape.h"
#include "Object.h"

namespace GFX::Shape
{
	class Ball : public BaseShape, public Object
	{
	public:
		Ball(Graphics& gfx, const DirectX::XMFLOAT3& position, const std::string& name, Data::ColorFloat4 material, unsigned int density, float radius);
		Ball(const Ball&) = delete;
		Ball& operator=(const Ball&) = delete;
		virtual ~Ball() = default;

		inline void SetTopologyMesh(Graphics& gfx) noexcept override { SetTopology(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ); }

		void Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override;
	};
}