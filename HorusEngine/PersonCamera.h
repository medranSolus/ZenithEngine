#pragma once
#include "BaseCamera.h"

namespace Camera
{
	class PersonCamera : public BaseCamera
	{
		DirectX::XMFLOAT3 moveDirection = { 0.0f, 0.0f, 1.0f };
		DirectX::XMFLOAT3 eyeDirection = { 0.0f, 0.0f, 1.0f };

		DirectX::FXMMATRIX UpdateView() const noexcept override;

	public:
		PersonCamera(GFX::Graphics& gfx, GFX::Pipeline::RenderGraph& graph, const std::string& name, float fov,
			float nearClip, float farClip, short angleHorizontal, short angleVertical, const DirectX::XMFLOAT3& position) noexcept;
		PersonCamera(const PersonCamera&) = default;
		PersonCamera& operator=(const PersonCamera&) = default;
		virtual ~PersonCamera() = default;

		void MoveX(float dX) noexcept override;
		inline void MoveY(float dY) noexcept override { position.y += dY; viewUpdate = true; }
		void MoveZ(float dZ) noexcept override;

		void Rotate(float angleDX, float angleDY) noexcept override;
	};
}