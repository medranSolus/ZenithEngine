#pragma once
#include "IShape.h"
#include "ProjectionData.h"

namespace GFX::Shape
{
	class CameraFrustum : public IShape
	{
	public:
		CameraFrustum(Graphics& gfx, Pipeline::RenderGraph& graph, const DirectX::XMFLOAT3& position,
			const std::string& name, Data::ColorFloat3 color, const Camera::ProjectionData& data);
		virtual ~CameraFrustum() = default;

		inline void SetTopologyPlain(Graphics& gfx) noexcept override {}

		void SetParams(Graphics& gfx, const Camera::ProjectionData& data);
	};
}