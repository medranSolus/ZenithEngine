#pragma once
#include "BaseCamera.h"

namespace Camera
{
	class FloatingCamera : public BaseCamera
	{
		Matrix UpdateView() const noexcept override;

	public:
		FloatingCamera(GFX::Graphics& gfx, GFX::Pipeline::RenderGraph& graph, CameraParams&& params) noexcept;
		FloatingCamera(FloatingCamera&&) = default;
		FloatingCamera(const FloatingCamera&) = default;
		FloatingCamera& operator=(FloatingCamera&&) = default;
		FloatingCamera& operator=(const FloatingCamera&) = default;
		virtual ~FloatingCamera() = default;

		void MoveX(float dX) noexcept override;
		void MoveY(float dY) noexcept override;

		void Rotate(float angleDX, float angleDY) noexcept override;
	};
}