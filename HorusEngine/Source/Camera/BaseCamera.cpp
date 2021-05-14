#include "Camera/BaseCamera.h"

namespace Camera
{
	GFX::Data::CBuffer::DCBLayout BaseCamera::MakeLayoutPS() noexcept
	{
		static GFX::Data::CBuffer::DCBLayout layout;
		static bool initNeeded = true;
		if (initNeeded)
		{
			layout.Add(DCBElementType::Matrix, "viewProjection");
			layout.Add(DCBElementType::Matrix, "inverseViewProjection");
			layout.Add(DCBElementType::Float3, "cameraPos");
			layout.Add(DCBElementType::Float, "nearClip");
			layout.Add(DCBElementType::Float, "farClip");
			initNeeded = false;
		}
		return layout;
	}

	Matrix BaseCamera::UpdateProjection() const noexcept
	{
		Matrix matrix = Math::XMMatrixPerspectiveFovLH(projection.fov, projection.screenRatio, projection.nearClip, projection.farClip);
		Math::XMStoreFloat4x4(&projectionMatrix, matrix);
		return matrix;
	}

	void BaseCamera::UpdateBufferVS(GFX::Graphics& gfx)
	{
		if (positionUpdate)
		{
			const Float3& pos = GetPos();
			positionBuffer->Update(gfx, { pos.x, pos.y, pos.z, 0.0f });
			positionUpdate = false;
		}
	}

	void BaseCamera::UpdateBufferPS()
	{
		const Matrix viewProjection = GetView() * GetProjection();
		Math::XMStoreFloat4x4(&cameraBuffer->GetBuffer()["viewProjection"], Math::XMMatrixTranspose(viewProjection));
		Math::XMStoreFloat4x4(&cameraBuffer->GetBuffer()["inverseViewProjection"],
			Math::XMMatrixTranspose(Math::XMMatrixInverse(nullptr, viewProjection)));
		cameraBuffer->GetBuffer()["nearClip"] = projection.nearClip;
		cameraBuffer->GetBuffer()["farClip"] = projection.farClip;
	}

	BaseCamera::BaseCamera(GFX::Graphics& gfx, GFX::Pipeline::RenderGraph& graph, CameraParams&& params) noexcept
		: ICamera(params.name), projection({ params.fov, gfx.GetRatio(), params.nearClip, params.farClip })
	{
		const std::string tag = "C_" + name;
		positionBuffer = GFX::Resource::ConstBufferVertex<Float4>::Get(gfx, tag, 1);
		cameraBuffer = GFX::Resource::ConstBufferExPixelCache::Get(gfx, tag, MakeLayoutPS(), 2);
		cameraBuffer->GetBuffer()["cameraPos"] = params.position;
		std::string name = params.name;
		indicator = std::make_unique<GFX::Shape::CameraIndicator>(gfx, graph, params.position, std::move(name), Float3(0.5f, 0.5f, 1.0f));
		frustum = std::make_unique<GFX::Shape::CameraFrustum>(gfx, graph, params.position, std::move(params.name), Float3(1.0f, 0.5f, 0.5f), projection);
	}

	Matrix BaseCamera::GetProjection() const noexcept
	{
		if (projectionUpdate)
		{
			projectionUpdate = false;
			return UpdateProjection();
		}
		else
			return Math::XMLoadFloat4x4(&projectionMatrix);
	}

	Matrix BaseCamera::GetView() const noexcept
	{
		if (viewUpdate)
		{
			viewUpdate = false;
			return UpdateView();
		}
		else
			return Math::XMLoadFloat4x4(&viewMatrix);
	}

	Math::BoundingFrustum BaseCamera::GetFrustum() const noexcept
	{
		Math::BoundingFrustum volume = GetProjection();
		volume.Transform(volume, 1.0f,
			Math::XMLoadFloat4(&indicator->GetAngle()),
			Math::XMLoadFloat3(&GetPos()));
		return volume;
	}

	void BaseCamera::MoveZ(float dZ) noexcept
	{
		Float3& position = cameraBuffer->GetBuffer()["cameraPos"];
		Math::XMStoreFloat3(&position,
			Math::XMVectorAdd(Math::XMLoadFloat3(&position),
				Math::XMVectorScale(Math::XMLoadFloat3(&moveDirection), dZ)));
		viewUpdate = true;
	}

	void BaseCamera::Roll(float delta) noexcept
	{
		const Vector rotor = Math::XMQuaternionRotationNormal(Math::XMLoadFloat3(&moveDirection), delta);
		Math::XMStoreFloat3(&up, Math::XMVector3Rotate(Math::XMLoadFloat3(&up), rotor));
		indicator->UpdateAngle(rotor);
		frustum->UpdateAngle(rotor);
		viewUpdate = true;
	}

	bool BaseCamera::Accept(GFX::Graphics& gfx, GFX::Probe::BaseProbe& probe) noexcept
	{
		ImGui::Checkbox("Indicator", &enableIndicator);
		if (enableIndicator)
			ImGui::Checkbox("Frustum", &enableFrustum);
		ImGui::NextColumn();
		projectionUpdate |= probe.VisitCamera(projection);
		if (projectionUpdate)
			frustum->SetParams(gfx, projection);
		//indicator->Accept(gfx, probe);
		return projectionUpdate;
	}

	void BaseCamera::Submit(U64 channelFilter) const noexcept
	{
		if (enableIndicator)
		{
			const Float3& position = cameraBuffer->GetBufferConst()["cameraPos"];
			indicator->SetPos(position);
			indicator->Submit(channelFilter);
			if (enableFrustum)
			{
				frustum->SetPos(position);
				frustum->Submit(channelFilter);
			}
		}
	}
}