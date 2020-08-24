#pragma once
#include "BaseShape.h"
#include "Object.h"

namespace GFX::Shape
{
	class CameraIndicator : public BaseShape, public Object
	{
	public:
		CameraIndicator(Graphics& gfx, Pipeline::RenderGraph& graph, const DirectX::XMFLOAT3& position, Data::ColorFloat3 color);
		CameraIndicator(const CameraIndicator&) = delete;
		CameraIndicator& operator=(const CameraIndicator&) = delete;
		virtual ~CameraIndicator() = default;

		inline void SetTopologyPlain(Graphics& gfx) noexcept override {}

		void Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override;
	};
}