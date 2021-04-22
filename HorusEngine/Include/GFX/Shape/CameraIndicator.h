#pragma once
#include "IShape.h"

namespace GFX::Shape
{
	class CameraIndicator : public IShape
	{
	public:
		CameraIndicator(Graphics& gfx, Pipeline::RenderGraph& graph,
			const Float3& position, std::string&& name, const ColorF3& color);
		virtual ~CameraIndicator() = default;

		constexpr void SetTopologyPlain(Graphics& gfx) noexcept override {}
	};
}