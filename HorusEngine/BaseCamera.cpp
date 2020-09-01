#include "BaseCamera.h"
#include "Math.h"

namespace Camera
{
	BaseCamera::BaseCamera(GFX::Graphics& gfx, GFX::Pipeline::RenderGraph& graph, const std::string& name,
		float fov, float nearClip, float farClip, const DirectX::XMFLOAT3& position) noexcept
		: ICamera(name), position(position), projection({ fov, gfx.GetRatio(), nearClip, farClip })
	{
		indicator = std::make_shared<GFX::Shape::CameraIndicator>(gfx, graph, position, name, DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f));
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
			indicator->SetPos(position);
			frustrum->SetPos(position);
		}
		if (projectionUpdate)
		{
			projectionUpdate = false;
			frustrum->SetParams(gfx, projection);
			gfx.GetProjection() = GetProjection();
		}
	}

	void BaseCamera::Accept(GFX::Graphics& gfx, GFX::Probe::BaseProbe& probe) noexcept
	{
		projectionUpdate = probe.VisitCamera(projection);
		//indicator->Accept(gfx, probe);
		//frustrum->Accept(gfx, probe);
	}
}