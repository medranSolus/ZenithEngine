#include "BaseCamera.h"
#include "Math.h"

namespace Camera
{
	DirectX::FXMMATRIX BaseCamera::GetView() const noexcept
	{
		if (viewUpdate)
			return UpdateView();
		return DirectX::XMLoadFloat4x4(&view);
	}

	void BaseCamera::Roll(float delta) noexcept
	{
		DirectX::XMStoreFloat3(&up,
			DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&up),
				DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, delta)));
		viewUpdate = true;
	}

	void BaseCamera::Update(GFX::Graphics& gfx) const noexcept
	{
		if (viewUpdate)
			gfx.GetCamera() = UpdateView();
		if (projectionUpdate)
		{
			projectionUpdate = false;
			gfx.GetProjection() = GetProjection();
		}
	}

	void BaseCamera::ShowWindow() noexcept
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
}