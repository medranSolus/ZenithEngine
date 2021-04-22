#include "Camera/PersonCamera.h"

namespace Camera
{
	Matrix PersonCamera::UpdateView() const noexcept
	{
		Matrix matrix = Math::XMMatrixLookToLH(Math::XMLoadFloat3(&cameraBuffer->GetBufferConst()["cameraPos"]),
			Math::XMLoadFloat3(&eyeDirection), Math::XMLoadFloat3(&up));
		Math::XMStoreFloat4x4(&viewMatrix, matrix);
		return matrix;
	}

	PersonCamera::PersonCamera(GFX::Graphics& gfx, GFX::Pipeline::RenderGraph& graph, CameraParams&& params) noexcept
		: BaseCamera(gfx, graph, std::forward<CameraParams>(params))
	{
		Rotate(params.angleVertical, params.angleHorizontal / gfx.GetRatio());
	}

	void PersonCamera::MoveX(float dX) noexcept
	{
		Float3& position = cameraBuffer->GetBuffer()["cameraPos"];
		Math::XMStoreFloat3(&position,
			Math::XMVectorAdd(Math::XMLoadFloat3(&position),
				Math::XMVectorScale(Math::XMVector3Cross({ 0.0f, 1.0f, 0.0f, 0.0f },
					Math::XMLoadFloat3(&moveDirection)), dX)));
		viewUpdate = true;
	}

	void PersonCamera::MoveY(float dY) noexcept
	{
		static_cast<Float3&>(cameraBuffer->GetBuffer()["cameraPos"]).y += dY;
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

		const Vector upV = Math::XMLoadFloat3(&up);
		const Vector eyeDirV = Math::XMLoadFloat3(&eyeDirection);
		Vector rotor = {};
		if (angleDX)
		{
			const float angle = Math::XMVectorGetX(Math::XMVector3AngleBetweenNormals(upV, eyeDirV)) + angleDX;
			if (angle <= FLIP_EPSILON || angle >= M_PI - FLIP_EPSILON)
			{
				angleDX = 0.0f;
				if (angleDY == 0.0f)
					return;
			}
			else
				rotor = Math::XMQuaternionRotationNormal(Math::XMVector3Cross(upV,
					Math::XMLoadFloat3(&moveDirection)), angleDX);
		}
		if (angleDY)
		{
			const Vector rotorY = Math::XMQuaternionRotationNormal(upV, angleDY);
			rotor = angleDX != 0.0f ? Math::XMQuaternionMultiply(rotor, rotorY) : rotorY;
		}

		// Unknown rotation when UP is strongly tilted, TODO: Perform some tests
		const Vector eyeV = Math::XMVector3Rotate(eyeDirV, rotor);
		Math::XMStoreFloat3(&eyeDirection, Math::XMVector3Normalize(eyeV));
		Math::XMStoreFloat3(&moveDirection, Math::XMVector3Normalize(Math::XMVectorSetY(eyeV, 0.0f)));

		indicator->UpdateAngle({ angleDX, angleDY, 0.0f });
		frustum->UpdateAngle({ angleDX, angleDY, 0.0f });
		viewUpdate = true;
	}
}