#pragma once
#include "BaseCamera.h"

namespace Camera
{
	class FloatingCamera : public BaseCamera
	{
		DirectX::XMMATRIX UpdateView() const noexcept override;

	public:
		FloatingCamera(GFX::Graphics& gfx, GFX::Pipeline::RenderGraph& graph, const CameraParams& params) noexcept;
		FloatingCamera(const FloatingCamera&) = default;
		FloatingCamera& operator=(const FloatingCamera&) = default;
		virtual ~FloatingCamera() = default;

		void MoveX(float dX) noexcept override;
		void MoveY(float dY) noexcept override;

		void Rotate(float angleDX, float angleDY) noexcept override;
	};
}