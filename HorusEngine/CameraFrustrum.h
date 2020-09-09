#pragma once
#include "BaseShape.h"
#include "Object.h"
#include "ProjectionData.h"

namespace GFX::Shape
{
	class CameraFrustrum : public BaseShape, public Object
	{
	public:
		CameraFrustrum(Graphics& gfx, Pipeline::RenderGraph& graph, const DirectX::XMFLOAT3& position,
			const std::string& name, Data::ColorFloat3 color, const Camera::ProjectionData& data);
		virtual ~CameraFrustrum() = default;

		inline void SetTopologyPlain(Graphics& gfx) noexcept override {}

		void Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override;
		void SetParams(Graphics& gfx, const Camera::ProjectionData& data);
	};
}