#pragma once
#include "IShape.h"
#include "Camera/ProjectionData.h"

namespace GFX::Shape
{
	class CameraFrustum : public IShape
	{
	public:
		CameraFrustum(Graphics& gfx, Pipeline::RenderGraph& graph, const Float3& position,
			std::string&& name, const ColorF3& color, const Camera::ProjectionData& data);
		virtual ~CameraFrustum() = default;

		constexpr void SetTopologyPlain(Graphics& gfx) noexcept override {}

		void SetParams(Graphics& gfx, const Camera::ProjectionData& data);
	};
}