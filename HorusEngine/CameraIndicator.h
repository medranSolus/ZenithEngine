#pragma once
#include "IShape.h"

namespace GFX::Shape
{
	class CameraIndicator : public IShape
	{
	public:
		CameraIndicator(Graphics& gfx, Pipeline::RenderGraph& graph, const DirectX::XMFLOAT3& position, const std::string& name, Data::ColorFloat3 color);
		virtual ~CameraIndicator() = default;

		inline void SetTopologyPlain(Graphics& gfx) noexcept override {}
	};
}