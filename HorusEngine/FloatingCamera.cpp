#include "FloatingCamera.h"
#include "Math.h"

namespace Camera
{
	DirectX::FXMMATRIX FloatingCamera::UpdateView() const noexcept
	{
		DirectX::XMMATRIX matrix = DirectX::XMMatrixLookToLH(DirectX::XMLoadFloat3(&position),
			DirectX::XMLoadFloat3(&moveDirection), DirectX::XMLoadFloat3(&up));
		DirectX::XMStoreFloat4x4(&viewMatrix, matrix);
		return std::move(matrix);
	}

	FloatingCamera::FloatingCamera(GFX::Graphics& gfx, GFX::Pipeline::RenderGraph& graph, const std::string& name, float fov,
		float nearClip, float farClip, short angleHorizontal, short angleVertical, const DirectX::XMFLOAT3& position) noexcept
		: BaseCamera(gfx, graph, name, fov, nearClip, farClip, position)
	{
		constexpr float pi = static_cast<float>(M_PI - FLT_EPSILON);
		Rotate(pi * static_cast<float>(Math::ClampAngle(angleVertical, 180)) / 361.0f,
			static_cast<float>(angleHorizontal) * pi / (180.0f * gfx.GetRatio()));
	}

	void FloatingCamera::MoveX(float dX) noexcept
	{
		float y = position.y;
		DirectX::XMStoreFloat3(&position,
			DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&position),
				DirectX::XMVectorScale(DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&moveDirection), DirectX::XMMatrixRotationY(M_PI_2)), dX)));
		position.y = y;
		viewUpdate = true;
	}

	void FloatingCamera::MoveY(float dY) noexcept
	{
		DirectX::XMStoreFloat3(&position,
			DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&position),
				DirectX::XMVectorScale(DirectX::XMLoadFloat3(&up), dY)));
		viewUpdate = true;
	}

	void FloatingCamera::MoveZ(float dZ) noexcept
	{
		DirectX::XMStoreFloat3(&position,
			DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&position),
				DirectX::XMVectorScale(DirectX::XMLoadFloat3(&moveDirection), dZ)));
		viewUpdate = true;
	}

	void FloatingCamera::Rotate(float angleDX, float angleDY) noexcept
	{
		constexpr float moveEpsilon = 0.001f - FLT_EPSILON;
		if (abs(angleDX) < moveEpsilon)
			angleDX = 0.0f;
		else
		{
			constexpr float flipEpsilon = 16.0f * FLT_EPSILON;
			const float angle = DirectX::XMVectorGetX(DirectX::XMVector3AngleBetweenNormals(DirectX::XMLoadFloat3(&up), DirectX::XMLoadFloat3(&moveDirection))) + angleDX;
			if (angle <= flipEpsilon || angle >= M_PI - flipEpsilon)
				angleDX = 0.0f;
		}
		if (abs(angleDY) < moveEpsilon)
			angleDY = 0.0f;
		if (angleDX || angleDY)
		{
			DirectX::XMStoreFloat3(&moveDirection,
				DirectX::XMVector3Normalize(DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&moveDirection),
					DirectX::XMMatrixRotationRollPitchYaw(moveDirection.z * angleDX, angleDY * projection.screenRatio, moveDirection.x * angleDX * -1.0f))));
			indicator->UpdateAngle({ angleDX, angleDY * projection.screenRatio, 0.0f });
			frustrum->UpdateAngle({ angleDX, angleDY * projection.screenRatio, 0.0f });
			viewUpdate = true;
		}
	}
}