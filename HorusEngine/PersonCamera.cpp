#include "PersonCamera.h"
#include "Math.h"

namespace Camera
{
	DirectX::FXMMATRIX PersonCamera::UpdateView() const noexcept
	{
		DirectX::XMMATRIX matrix = DirectX::XMMatrixLookToLH(DirectX::XMLoadFloat3(&cameraBuffer->GetBufferConst()["cameraPos"]),
			DirectX::XMLoadFloat3(&eyeDirection), DirectX::XMLoadFloat3(&up));
		DirectX::XMStoreFloat4x4(&viewMatrix, matrix);
		return std::move(matrix);
	}

	PersonCamera::PersonCamera(GFX::Graphics& gfx, GFX::Pipeline::RenderGraph& graph, const std::string& name, float fov,
		float nearClip, float farClip, short angleHorizontal, short angleVertical, const DirectX::XMFLOAT3& position) noexcept
		: BaseCamera(gfx, graph, name, fov, nearClip, farClip, position)
	{
		constexpr float PI = static_cast<float>(M_PI - FLT_EPSILON);
		Rotate(PI * static_cast<float>(Math::ClampAngle(angleVertical, 180)) / 361.0f,
			static_cast<float>(angleHorizontal) * PI / (180.0f * gfx.GetRatio()));
	}

	void PersonCamera::MoveX(float dX) noexcept
	{
		DirectX::XMFLOAT3& position = cameraBuffer->GetBuffer()["cameraPos"];
		DirectX::XMStoreFloat3(&position,
			DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&position),
				DirectX::XMVectorScale(DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&moveDirection), DirectX::XMMatrixRotationY(M_PI_2)), dX)));
		viewUpdate = true;
	}

	void PersonCamera::MoveZ(float dZ) noexcept
	{
		DirectX::XMFLOAT3& position = cameraBuffer->GetBuffer()["cameraPos"];
		DirectX::XMStoreFloat3(&position,
			DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&position),
				DirectX::XMVectorScale(DirectX::XMLoadFloat3(&moveDirection), dZ)));
		viewUpdate = true;
	}

	void PersonCamera::Rotate(float angleDX, float angleDY) noexcept
	{
		constexpr float MOVE_EPSILON = 0.001f - FLT_EPSILON;
		if (abs(angleDX) < MOVE_EPSILON)
			angleDX = 0.0f;
		else
		{
			constexpr float FLIP_EPSILON = 16.0f * FLT_EPSILON;
			const float angle = DirectX::XMVectorGetX(DirectX::XMVector3AngleBetweenNormals(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),
				DirectX::XMLoadFloat3(&eyeDirection))) + angleDX;
			if (angle <= FLIP_EPSILON || angle >= M_PI - FLIP_EPSILON)
				angleDX = 0.0f;
		}
		if (abs(angleDY) < MOVE_EPSILON)
			angleDY = 0.0f;
		if (angleDX || angleDY)
		{
			DirectX::XMVECTOR vForward = DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&eyeDirection),
				DirectX::XMMatrixRotationRollPitchYaw(moveDirection.z * angleDX, angleDY * projection.screenRatio, moveDirection.x * angleDX * -1.0f));
			DirectX::XMStoreFloat3(&eyeDirection, DirectX::XMVector3Normalize(vForward));
			DirectX::XMStoreFloat3(&moveDirection, DirectX::XMVector3Normalize(DirectX::XMVectorSetY(vForward, 0.0f)));
			indicator->UpdateAngle({ angleDX, angleDY * projection.screenRatio, 0.0f });
			frustrum->UpdateAngle({ angleDX, angleDY * projection.screenRatio, 0.0f });
			viewUpdate = true;
		}
	}
}