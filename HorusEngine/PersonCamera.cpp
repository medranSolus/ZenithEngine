#include "PersonCamera.h"
#include "Math.h"

namespace Camera
{
	DirectX::XMMATRIX PersonCamera::UpdateView() const noexcept
	{
		DirectX::XMMATRIX matrix = DirectX::XMMatrixLookToLH(DirectX::XMLoadFloat3(&cameraBuffer->GetBufferConst()["cameraPos"]),
			DirectX::XMLoadFloat3(&eyeDirection), DirectX::XMLoadFloat3(&up));
		DirectX::XMStoreFloat4x4(&viewMatrix, matrix);
		return std::move(matrix);
	}

	PersonCamera::PersonCamera(GFX::Graphics& gfx, GFX::Pipeline::RenderGraph& graph, const CameraParams& params) noexcept
		: BaseCamera(gfx, graph, params)
	{
		Rotate(params.angleVertical, params.angleHorizontal / gfx.GetRatio());
	}

	void PersonCamera::MoveX(float dX) noexcept
	{
		DirectX::XMFLOAT3& position = cameraBuffer->GetBuffer()["cameraPos"];
		DirectX::XMStoreFloat3(&position,
			DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&position),
				DirectX::XMVectorScale(DirectX::XMVector3Cross({ 0.0f, 1.0f, 0.0f, 0.0f },
					DirectX::XMLoadFloat3(&moveDirection)), dX)));
		viewUpdate = true;
	}

	void PersonCamera::Rotate(float angleDX, float angleDY) noexcept
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
		const DirectX::XMVECTOR eyeDirV = DirectX::XMLoadFloat3(&eyeDirection);
		DirectX::XMVECTOR rotor = {};
		if (angleDX)
		{
			const float angle = DirectX::XMVectorGetX(DirectX::XMVector3AngleBetweenNormals(upV, eyeDirV)) + angleDX;
			if (angle <= FLIP_EPSILON || angle >= M_PI - FLIP_EPSILON)
			{
				angleDX = 0.0f;
				if (angleDY == 0.0f)
					return;
			}
			else
				rotor = DirectX::XMQuaternionRotationNormal(DirectX::XMVector3Cross(upV,
					DirectX::XMLoadFloat3(&moveDirection)), angleDX);
		}
		if (angleDY)
		{
			const DirectX::XMVECTOR rotorY = DirectX::XMQuaternionRotationNormal(upV, angleDY);
			rotor = angleDX != 0.0f ? DirectX::XMQuaternionMultiply(rotor, rotorY) : rotorY;
		}

		// Unknown rotation when UP is strongly tilted, TODO: Perform some tests
		const DirectX::XMVECTOR eyeV = DirectX::XMVector3Rotate(eyeDirV, rotor);
		DirectX::XMStoreFloat3(&eyeDirection, DirectX::XMVector3Normalize(eyeV));
		DirectX::XMStoreFloat3(&moveDirection, DirectX::XMVector3Normalize(DirectX::XMVectorSetY(eyeV, 0.0f)));

		indicator->UpdateAngle({ angleDX, angleDY, 0.0f });
		frustum->UpdateAngle({ angleDX, angleDY, 0.0f });
		viewUpdate = true;
	}
}