#include "Camera/FloatingCamera.h"

namespace Camera
{
	Matrix FloatingCamera::UpdateView() const noexcept
	{
		Matrix matrix = Math::XMMatrixLookToLH(Math::XMLoadFloat3(&cameraBuffer->GetBufferConst()["cameraPos"]),
			Math::XMLoadFloat3(&moveDirection), Math::XMLoadFloat3(&up));
		Math::XMStoreFloat4x4(&viewMatrix, matrix);
		return matrix;
	}

	FloatingCamera::FloatingCamera(GFX::Graphics& gfx, GFX::Pipeline::RenderGraph& graph, CameraParams&& params) noexcept
		: BaseCamera(gfx, graph, std::forward<CameraParams>(params))
	{
		Rotate(params.angleVertical, params.angleHorizontal / gfx.GetRatio());
	}

	void FloatingCamera::MoveX(float dX) noexcept
	{
		Float3& position = cameraBuffer->GetBuffer()["cameraPos"];
		Math::XMStoreFloat3(&position,
			Math::XMVectorAdd(Math::XMLoadFloat3(&position),
				Math::XMVectorScale(Math::XMVector3Cross(Math::XMLoadFloat3(&up),
					Math::XMLoadFloat3(&moveDirection)), dX)));
		viewUpdate = true;
	}

	void FloatingCamera::MoveY(float dY) noexcept
	{
		Float3& position = cameraBuffer->GetBuffer()["cameraPos"];
		Math::XMStoreFloat3(&position,
			Math::XMVectorAdd(Math::XMLoadFloat3(&position),
				Math::XMVectorScale(Math::XMLoadFloat3(&up), dY)));
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

		const Vector upV = Math::XMLoadFloat3(&up);
		const Vector moveDirV = Math::XMLoadFloat3(&moveDirection);
		Vector rotor = {};
		if (angleDX)
		{
			rotor = Math::XMQuaternionRotationNormal(Math::XMVector3Cross(upV, moveDirV), angleDX);
			Math::XMStoreFloat3(&up, Math::XMVector3Normalize(Math::XMVector3Rotate(upV, rotor)));
		}
		if (angleDY)
		{
			const Vector rotorY = Math::XMQuaternionRotationNormal(upV, angleDY);
			rotor = angleDX != 0.0f ? Math::XMQuaternionMultiply(rotor, rotorY) : rotorY;
		}
		Math::XMStoreFloat3(&moveDirection,
			Math::XMVector3Normalize(Math::XMVector3Rotate(moveDirV, rotor)));

		indicator->UpdateAngle({ angleDX, angleDY, 0.0f });
		frustum->UpdateAngle({ angleDX, angleDY, 0.0f });
		viewUpdate = true;
	}
}