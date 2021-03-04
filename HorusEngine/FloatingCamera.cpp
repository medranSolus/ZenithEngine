#include "FloatingCamera.h"
#include "Math.h"

namespace Camera
{
	DirectX::XMMATRIX FloatingCamera::UpdateView() const noexcept
	{
		DirectX::XMMATRIX matrix = DirectX::XMMatrixLookToLH(DirectX::XMLoadFloat3(&cameraBuffer->GetBufferConst()["cameraPos"]),
			DirectX::XMLoadFloat3(&moveDirection), DirectX::XMLoadFloat3(&up));
		DirectX::XMStoreFloat4x4(&viewMatrix, matrix);
		return std::move(matrix);
	}

	FloatingCamera::FloatingCamera(GFX::Graphics& gfx, GFX::Pipeline::RenderGraph& graph, const CameraParams& params) noexcept
		: BaseCamera(gfx, graph, params)
	{
		Rotate(params.angleVertical, params.angleHorizontal / gfx.GetRatio());
	}

	void FloatingCamera::MoveX(float dX) noexcept
	{
		DirectX::XMFLOAT3& position = cameraBuffer->GetBuffer()["cameraPos"];
		DirectX::XMStoreFloat3(&position,
			DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&position),
				DirectX::XMVectorScale(DirectX::XMVector3Cross(DirectX::XMLoadFloat3(&up),
					DirectX::XMLoadFloat3(&moveDirection)), dX)));
		viewUpdate = true;
	}

	void FloatingCamera::MoveY(float dY) noexcept
	{
		DirectX::XMFLOAT3& position = cameraBuffer->GetBuffer()["cameraPos"];
		DirectX::XMStoreFloat3(&position,
			DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&position),
				DirectX::XMVectorScale(DirectX::XMLoadFloat3(&up), dY)));
		viewUpdate = true;
	}

	void FloatingCamera::Rotate(float angleDX, float angleDY) noexcept
	{
		angleDY *= projection.screenRatio;
		if (abs(angleDX) < MOVE_EPSILON)
			angleDX = 0.0f;
		if (abs(angleDY) < MOVE_EPSILON)
		{
			angleDY = 0.0f;
			if (angleDX == 0.0f)
				return;
		}

		const DirectX::XMVECTOR upV = DirectX::XMLoadFloat3(&up);
		const DirectX::XMVECTOR moveDirV = DirectX::XMLoadFloat3(&moveDirection);
		DirectX::XMVECTOR rotor = {};
		if (angleDX)
		{
			rotor = DirectX::XMQuaternionRotationNormal(DirectX::XMVector3Cross(upV, moveDirV), angleDX);
			DirectX::XMStoreFloat3(&up,
				DirectX::XMVector3Normalize(DirectX::XMVector3Rotate(upV, rotor)));
		}
		if (angleDY)
		{
			const DirectX::XMVECTOR rotorY = DirectX::XMQuaternionRotationNormal(upV, angleDY);
			rotor = angleDX != 0.0f ? DirectX::XMQuaternionMultiply(rotor, rotorY) : rotorY;
		}
		DirectX::XMStoreFloat3(&moveDirection,
			DirectX::XMVector3Normalize(DirectX::XMVector3Rotate(moveDirV, rotor)));

		indicator->UpdateAngle({ angleDX, angleDY, 0.0f });
		frustum->UpdateAngle({ angleDX, angleDY, 0.0f });
		viewUpdate = true;
	}
}