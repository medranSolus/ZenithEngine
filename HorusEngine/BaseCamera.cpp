#include "BaseCamera.h"
#include "Math.h"

namespace Camera
{
	inline GFX::Data::CBuffer::DCBLayout BaseCamera::MakeLayoutPS() noexcept
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

	DirectX::XMMATRIX BaseCamera::UpdateProjection() const noexcept
	{
		DirectX::XMMATRIX matrix = DirectX::XMMatrixPerspectiveFovLH(projection.fov, projection.screenRatio, projection.nearClip, projection.farClip);
		DirectX::XMStoreFloat4x4(&projectionMatrix, matrix);
		return matrix;
	}

	void BaseCamera::UpdateBufferVS(GFX::Graphics& gfx) noexcept
	{
		if (positionUpdate)
		{
			const DirectX::XMFLOAT3& pos = GetPos();
			positionBuffer->Update(gfx, { pos.x, pos.y, pos.z, 0.0f });
		}
	}

	void BaseCamera::UpdateBufferPS() noexcept
	{
		const DirectX::XMMATRIX viewProjection = GetView() * GetProjection();
		DirectX::XMStoreFloat4x4(&cameraBuffer->GetBuffer()["viewProjection"], DirectX::XMMatrixTranspose(viewProjection));
		DirectX::XMStoreFloat4x4(&cameraBuffer->GetBuffer()["inverseViewProjection"],
			DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, viewProjection)));
		cameraBuffer->GetBuffer()["nearClip"] = projection.nearClip;
		cameraBuffer->GetBuffer()["farClip"] = projection.farClip;
	}

	BaseCamera::BaseCamera(GFX::Graphics& gfx, GFX::Pipeline::RenderGraph& graph, const CameraParams& params) noexcept
		: ICamera(params.name), projection({ params.fov, gfx.GetRatio(), params.nearClip, params.farClip })
	{
		positionBuffer = GFX::Resource::ConstBufferVertex<DirectX::XMFLOAT4>::Get(gfx, typeid(BaseCamera).name() + name, 1U);
		cameraBuffer = GFX::Resource::ConstBufferExPixelCache::Get(gfx, typeid(BaseCamera).name() + name, MakeLayoutPS(), 2U);
		cameraBuffer->GetBuffer()["cameraPos"] = params.position;
		indicator = std::make_unique<GFX::Shape::CameraIndicator>(gfx, graph, params.position, name, DirectX::XMFLOAT3(0.5f, 0.5f, 1.0f));
		frustum = std::make_unique<GFX::Shape::CameraFrustum>(gfx, graph, params.position, name, DirectX::XMFLOAT3(1.0f, 0.5f, 0.5f), projection);
	}

	DirectX::XMMATRIX BaseCamera::GetProjection() const noexcept
	{
		if (projectionUpdate)
		{
			projectionUpdate = false;
			return UpdateProjection();
		}
		else
			return DirectX::XMLoadFloat4x4(&projectionMatrix);
	}

	DirectX::XMMATRIX BaseCamera::GetView() const noexcept
	{
		if (viewUpdate)
		{
			viewUpdate = false;
			return UpdateView();
		}
		else
			return DirectX::XMLoadFloat4x4(&viewMatrix);
	}

	DirectX::BoundingFrustum BaseCamera::GetFrustum() const noexcept
	{
		DirectX::BoundingFrustum volume = GetProjection();
		volume.Transform(volume, 1.0f,
			DirectX::XMQuaternionRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&indicator->GetAngle())),
			DirectX::XMLoadFloat3(&GetPos()));
		return volume;
	}

	void BaseCamera::MoveZ(float dZ) noexcept
	{
		DirectX::XMFLOAT3& position = cameraBuffer->GetBuffer()["cameraPos"];
		DirectX::XMStoreFloat3(&position,
			DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&position),
				DirectX::XMVectorScale(DirectX::XMLoadFloat3(&moveDirection), dZ)));
		viewUpdate = true;
	}

	void BaseCamera::Roll(float delta) noexcept
	{
		DirectX::XMStoreFloat3(&up,
			DirectX::XMVector3Rotate(DirectX::XMLoadFloat3(&up),
				DirectX::XMQuaternionRotationNormal(DirectX::XMLoadFloat3(&moveDirection), delta)));
		indicator->UpdateAngle({ 0.0f, 0.0f, delta });
		frustum->UpdateAngle({ 0.0f, 0.0f, delta });
		viewUpdate = true;
	}

	void BaseCamera::BindCamera(GFX::Graphics& gfx) const noexcept
	{
		gfx.SetView(GetView());
		gfx.SetProjection(GetProjection());
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

	void BaseCamera::Submit(uint64_t channelFilter) noexcept
	{
		if (enableIndicator)
		{
			const DirectX::XMFLOAT3& position = cameraBuffer->GetBufferConst()["cameraPos"];
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