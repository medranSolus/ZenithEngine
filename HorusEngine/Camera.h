#pragma once
#include <DirectXMath.h>

class Camera
{
	float angleZ = 0.0f;
	float angleX = 0.0f;
	float angleY = 0.0f;
	float x;
	float y;
	float z;

public:
	Camera(float x0 = 0.0f, float y0 = 0.0f, float z0 = 0.0f) : x(x0), y(y0), z(z0) {}

	DirectX::XMMATRIX GetMatrix() const noexcept;
	void Move(float dX, float dY, float dZ) noexcept;
	void Rotate(float dZ, float dX, float dY) noexcept;
};