#include "BaseCamera.h"
#include "Math.h"

namespace Camera
{
	BaseCamera::BaseCamera(GFX::Graphics& gfx, GFX::Pipeline::RenderGraph& graph, const std::string& name,
		float fov, float nearClip, float farClip, const DirectX::XMFLOAT3& position) noexcept
		: ICamera(name), position(position), projection({ fov, gfx.GetRatio(), nearClip, farClip })
	{
		indicator = std::make_shared<GFX::Shape::CameraIndicator>(gfx, graph, position, name, DirectX::XMFLOAT3(0.5f, 0.5f, 1.0f));
		frustrum = std::make_shared<GFX::Shape::CameraFrustrum>(gfx, graph, position, name, DirectX::XMFLOAT3(1.0f, 0.5f, 0.5f), projection);
	}

	void BaseCamera::Roll(float delta) noexcept
	{
		DirectX::XMStoreFloat3(&up,
			DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&up),
				DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, delta)));
		indicator->UpdateAngle({ 0.0f, 0.0f, delta });
		frustrum->UpdateAngle({ 0.0f, 0.0f, delta });
		viewUpdate = true;
	}

	void BaseCamera::Update(GFX::Graphics& gfx) const noexcept
	{
		if (viewUpdate)
		{
			gfx.GetCamera() = UpdateView();
			if (enableIndicator)
			{
				indicator->SetPos(position);
				if (enableFrustrum)
					frustrum->SetPos(position);
			}
		}
		if (projectionUpdate)
		{
			projectionUpdate = false;
			gfx.GetProjection() = GetProjection();
		}
	}

	void BaseCamera::Accept(GFX::Graphics& gfx, GFX::Probe::BaseProbe& probe) noexcept
	{
		ImGui::Checkbox("Enable Indicator", &enableIndicator);
		if (enableIndicator)
		{
			ImGui::SameLine();
			ImGui::Checkbox("Enable Frustrum", &enableFrustrum);
		}
		projectionUpdate |= probe.VisitCamera(projection);
		if (projectionUpdate)
			frustrum->SetParams(gfx, projection);
		//indicator->Accept(gfx, probe);
	}

	void BaseCamera::Submit(uint64_t channelFilter) noexcept
	{
		if (enableIndicator)
		{
			indicator->Submit(channelFilter);
			if (enableFrustrum)
				frustrum->Submit(channelFilter);
		}
	}
}