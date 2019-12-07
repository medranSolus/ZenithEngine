#include "Camera.h"
#include "Math.h"

Camera::Camera(float screenRatio, float nearClip, float farClip, float x0, float y0, float z0) noexcept
	: screenRatio(screenRatio), nearClip(nearClip), farClip(farClip)
{
	position = DirectX::XMFLOAT3(x0, y0, z0);
	moveDirection = eyeDirection = DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);
	up = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
}

void Camera::MoveX(float dX) noexcept
{
	float y = position.y;
	DirectX::XMStoreFloat3(&position,
		DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&position),
			DirectX::XMVectorScale(DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&moveDirection), DirectX::XMMatrixRotationY(M_PI_2)), dX)));
	position.y = y;
}

void Camera::MoveZ(float dZ) noexcept
{
	float y = position.y;
	DirectX::XMStoreFloat3(&position,
		DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&position),
			DirectX::XMVectorScale(DirectX::XMLoadFloat3(&moveDirection), dZ)));
	position.y = y;
}

void Camera::Rotate(float angleDX, float angleDY) noexcept
{
	constexpr float moveEpsilon = 0.001f - FLT_EPSILON;
	if (abs(angleDX) < moveEpsilon)
		angleDX = 0.0f;
	if (abs(angleDY) < moveEpsilon)
		angleDY = 0.0f;
	if (angleDX || angleDY)
	{
		constexpr float flipEpsilon = 16.0f * FLT_EPSILON;
		float angle = DirectX::XMVectorGetX(DirectX::XMVector3AngleBetweenNormals(DirectX::XMLoadFloat3(&up), DirectX::XMLoadFloat3(&eyeDirection))) + angleDX;
		if (angle <= flipEpsilon || angle >= M_PI - flipEpsilon)
			angleDX = 0.0f;
		DirectX::XMVECTOR vForward = DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&eyeDirection),
			DirectX::XMMatrixRotationRollPitchYaw(moveDirection.z * angleDX, angleDY * screenRatio, moveDirection.x * angleDX * -1.0f));
		DirectX::XMStoreFloat3(&eyeDirection, DirectX::XMVector3Normalize(vForward));
		DirectX::XMStoreFloat3(&moveDirection, DirectX::XMVector3Normalize(DirectX::XMVectorSetY(vForward, 0.0f)));
	}
}

DirectX::XMMATRIX Camera::GetView() const noexcept
{
	return DirectX::XMMatrixLookToLH(DirectX::XMLoadFloat3(&position),
		DirectX::XMLoadFloat3(&eyeDirection), DirectX::XMLoadFloat3(&up));
}

DirectX::XMMATRIX Camera::GetProjection() const noexcept
{
	return DirectX::XMMatrixPerspectiveFovLH(45.0f, screenRatio, nearClip, farClip);
}
