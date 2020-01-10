#include "PersonCamera.h"

namespace Camera
{
	DirectX::FXMMATRIX PersonCamera::UpdateView() const noexcept
	{
		DirectX::XMMATRIX matrix = DirectX::XMMatrixLookToLH(DirectX::XMLoadFloat3(&position),
			DirectX::XMLoadFloat3(&eyeDirection), DirectX::XMLoadFloat3(&up));
		DirectX::XMStoreFloat4x4(&view, matrix);
		viewUpdate = false;
		return std::move(matrix);
	}

	void PersonCamera::MoveX(float dX) noexcept
	{
		DirectX::XMStoreFloat3(&position,
			DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&position),
				DirectX::XMVectorScale(DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&moveDirection), DirectX::XMMatrixRotationY(M_PI_2)), dX)));
		viewUpdate = true;
	}

	void PersonCamera::MoveZ(float dZ) noexcept
	{
		DirectX::XMStoreFloat3(&position,
			DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&position),
				DirectX::XMVectorScale(DirectX::XMLoadFloat3(&moveDirection), dZ)));
		viewUpdate = true;
	}

	void PersonCamera::Rotate(float angleDX, float angleDY) noexcept
	{
		constexpr float moveEpsilon = 0.001f - FLT_EPSILON;
		if (abs(angleDX) < moveEpsilon)
			angleDX = 0.0f;
		else
		{
			constexpr float flipEpsilon = 16.0f * FLT_EPSILON;
			const float angle = DirectX::XMVectorGetX(DirectX::XMVector3AngleBetweenNormals(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),
				DirectX::XMLoadFloat3(&eyeDirection))) + angleDX;
			if (angle <= flipEpsilon || angle >= M_PI - flipEpsilon)
				angleDX = 0.0f;
		}
		if (abs(angleDY) < moveEpsilon)
			angleDY = 0.0f;
		if (angleDX || angleDY)
		{
			DirectX::XMVECTOR vForward = DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&eyeDirection),
				DirectX::XMMatrixRotationRollPitchYaw(moveDirection.z * angleDX, angleDY * screenRatio, moveDirection.x * angleDX * -1.0f));
			DirectX::XMStoreFloat3(&eyeDirection, DirectX::XMVector3Normalize(vForward));
			DirectX::XMStoreFloat3(&moveDirection, DirectX::XMVector3Normalize(DirectX::XMVectorSetY(vForward, 0.0f)));
			viewUpdate = true;
		}
	}
}
