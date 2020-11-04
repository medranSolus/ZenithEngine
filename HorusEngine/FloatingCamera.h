#pragma once
#include "BaseCamera.h"

namespace Camera
{
	class FloatingCamera : public BaseCamera
	{
		DirectX::XMFLOAT3 moveDirection = { 0.0f, 0.0f, 1.0f };

		DirectX::XMMATRIX UpdateView() const noexcept override;

	public:
		FloatingCamera(GFX::Graphics& gfx, GFX::Pipeline::RenderGraph& graph, const std::string& name, float fov,
			float nearClip, float farClip, short angleHorizontal, short angleVertical, const DirectX::XMFLOAT3& position) noexcept;
		FloatingCamera(const FloatingCamera&) = default;
		FloatingCamera& operator=(const FloatingCamera&) = default;
		virtual ~FloatingCamera() = default;

		void MoveX(float dX) noexcept override;
		void MoveY(float dY) noexcept override;
		void MoveZ(float dZ) noexcept override;

		void Rotate(float angleDX, float angleDY) noexcept override;
	};
}