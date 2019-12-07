#pragma once
#include <DirectXMath.h>

class Camera
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 eyeDirection;
	DirectX::XMFLOAT3 moveDirection;
	DirectX::XMFLOAT3 up;
	float screenRatio;
	float nearClip;
	float farClip;

public:
	Camera(float screenRatio, float nearClip, float farClip, float x0 = 0.0f, float y0 = 0.0f, float z0 = -1.0f) noexcept;

	constexpr  void MoveY(float dY) noexcept { position.y += dY; }

	void MoveX(float dX) noexcept;
	void MoveZ(float dZ) noexcept;
	void Rotate(float angleDX, float angleDY) noexcept;
	DirectX::XMMATRIX GetView() const noexcept;
	DirectX::XMMATRIX GetProjection() const noexcept;
};