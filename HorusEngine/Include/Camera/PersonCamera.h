#pragma once
#include "BaseCamera.h"

namespace Camera
{
	class PersonCamera : public BaseCamera
	{
		Float3 eyeDirection = { 0.0f, 0.0f, 1.0f };

		Matrix UpdateView() const noexcept override;

	public:
		PersonCamera(GFX::Graphics& gfx, GFX::Pipeline::RenderGraph& graph, CameraParams&& params) noexcept;
		PersonCamera(PersonCamera&&) = default;
		PersonCamera(const PersonCamera&) = default;
		PersonCamera& operator=(PersonCamera&&) = default;
		PersonCamera& operator=(const PersonCamera&) = default;
		virtual ~PersonCamera() = default;

		void MoveX(float dX) noexcept override;
		void MoveY(float dY) noexcept override;

		void Rotate(float angleDX, float angleDY) noexcept override;
	};
}