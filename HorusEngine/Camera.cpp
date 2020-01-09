#include "Camera.h"
#include "Math.h"
#include "ImGui/imgui.h"

Camera::Camera(float fov, float screenRatio, float nearClip, float farClip, float x0, float y0, float z0) noexcept
	: fov(fov), screenRatio(screenRatio), nearClip(nearClip), farClip(farClip)
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
	viewUpdate = true;
}

void Camera::MoveZ(float dZ) noexcept
{
	float y = position.y;
	DirectX::XMStoreFloat3(&position,
		DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&position),
			DirectX::XMVectorScale(DirectX::XMLoadFloat3(&moveDirection), dZ)));
	position.y = y;
	viewUpdate = true;
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
		float angle = DirectX::XMVectorGetX(DirectX::XMVector3AngleBetweenNormals(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), DirectX::XMLoadFloat3(&eyeDirection))) + angleDX;
		if (angle <= flipEpsilon || angle >= M_PI - flipEpsilon)
			angleDX = 0.0f;
		DirectX::XMVECTOR vForward = DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&eyeDirection),
			DirectX::XMMatrixRotationRollPitchYaw(moveDirection.z * angleDX, angleDY * screenRatio, moveDirection.x * angleDX * -1.0f));
		DirectX::XMStoreFloat3(&eyeDirection, DirectX::XMVector3Normalize(vForward));
		DirectX::XMStoreFloat3(&moveDirection, DirectX::XMVector3Normalize(DirectX::XMVectorSetY(vForward, 0.0f)));
		viewUpdate = true;
	}
}

void Camera::Roll(float delta) noexcept
{
	DirectX::XMStoreFloat3(&up, DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&up), DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, delta)));
	viewUpdate = true;
}

void Camera::Update(GFX::Graphics & gfx) const noexcept
{
	if (viewUpdate)
		gfx.GetCamera() = GetView();
	if (projectionUpdate)
		gfx.GetProjection() = GetProjection();
}

void Camera::UpdateView() const noexcept
{
	if (viewUpdate)
	{
		DirectX::XMStoreFloat4x4(&view, DirectX::XMMatrixLookToLH(DirectX::XMLoadFloat3(&position),
			DirectX::XMLoadFloat3(&eyeDirection), DirectX::XMLoadFloat3(&up)));
		viewUpdate = false;
	}
}

void Camera::UpdateProjection() const noexcept
{
	if (projectionUpdate)
	{
		DirectX::XMStoreFloat4x4(&projection, DirectX::XMMatrixPerspectiveFovLH(fov, screenRatio, nearClip, farClip));
		projectionUpdate = false;
	}
}

DirectX::FXMMATRIX Camera::GetView() const noexcept
{
	if (viewUpdate)
	{
		DirectX::XMMATRIX matrix = DirectX::XMMatrixLookToLH(DirectX::XMLoadFloat3(&position),
			DirectX::XMLoadFloat3(&eyeDirection), DirectX::XMLoadFloat3(&up));
		DirectX::XMStoreFloat4x4(&view, matrix);
		viewUpdate = false;
		return matrix;
	}
	return DirectX::XMLoadFloat4x4(&view);
}

DirectX::FXMMATRIX Camera::GetProjection() const noexcept
{
	if (projectionUpdate)
	{
		DirectX::XMMATRIX matrix = DirectX::XMMatrixPerspectiveFovLH(fov, screenRatio, nearClip, farClip);
		DirectX::XMStoreFloat4x4(&projection, matrix);
		projectionUpdate = false;
		return matrix;
	}
	return DirectX::XMLoadFloat4x4(&projection);
}

void Camera::ShowWindow() noexcept
{
	float oldFov = fov;
	float oldNearClip = nearClip;
	float oldFarClip = farClip;
	float oldScreenRatio = screenRatio;
	ImGui::SliderAngle("FOV", &fov, 1.0f, 179.0f, "%.1f");
	ImGui::SliderFloat("Near clip", &nearClip, 0.001f, 10.0f, "%.3f");
	ImGui::SliderFloat("Far clip", &farClip, 1.0f, 50000.0f, "%.1f");
	ImGui::SliderFloat("Ratio", &screenRatio, 0.1f, 5.0f, "%.2f");
	if (notEquals(oldFov, fov) || notEquals(oldNearClip, nearClip) || notEquals(oldFarClip, farClip) || notEquals(oldScreenRatio, screenRatio))
		projectionUpdate = true;
}
