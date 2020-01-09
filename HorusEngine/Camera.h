#pragma once
#include "Graphics.h"

class Camera
{
	mutable DirectX::XMFLOAT4X4 view;
	mutable DirectX::XMFLOAT4X4 projection;
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 eyeDirection;
	DirectX::XMFLOAT3 moveDirection;
	DirectX::XMFLOAT3 up;
	float fov;
	float screenRatio;
	float nearClip;
	float farClip;
	mutable bool viewUpdate = true;
	mutable bool projectionUpdate = true;

public:
	Camera(float fov, float screenRatio, float nearClip, float farClip, float x0 = 0.0f, float y0 = 0.0f, float z0 = -1.0f) noexcept;
	Camera(const Camera&) = default;
	Camera & operator=(const Camera&) = default;
	~Camera() = default;

	constexpr void MoveY(float dY) noexcept { position.y += dY; viewUpdate = true; }
	inline void UpdateMatrices() const noexcept { UpdateView(); UpdateProjection(); }

	void MoveX(float dX) noexcept;
	void MoveZ(float dZ) noexcept;
	void Rotate(float angleDX, float angleDY) noexcept;
	void Roll(float delta) noexcept;

	void Update(GFX::Graphics & gfx) const noexcept;
	void UpdateView() const noexcept;
	void UpdateProjection() const noexcept;
	DirectX::FXMMATRIX GetView() const noexcept;
	DirectX::FXMMATRIX GetProjection() const noexcept;

	void ShowWindow() noexcept;
};