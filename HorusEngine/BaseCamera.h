#pragma once
#include "ICamera.h"
#include "CameraIndicator.h"

namespace Camera
{
	class BaseCamera : public ICamera
	{
	protected:
		mutable DirectX::XMFLOAT4X4 view;
		ProjectionData projection;
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 up = { 0.0f, 1.0f, 0.0f };
		std::shared_ptr<GFX::Shape::CameraIndicator> indicator = nullptr;
		mutable bool viewUpdate = true;
		mutable bool projectionUpdate = true;

		virtual DirectX::FXMMATRIX UpdateView() const noexcept = 0;

	public:
		BaseCamera(GFX::Graphics& gfx, GFX::Pipeline::RenderGraph& graph, const std::string& name,
			float fov, float nearClip, float farClip, const DirectX::XMFLOAT3& position) noexcept;
		BaseCamera(const BaseCamera&) = default;
		BaseCamera& operator=(const BaseCamera&) = default;
		virtual ~BaseCamera() = default;

		inline void ResetView() const noexcept override { viewUpdate = true; }
		inline void ResetProjection() const noexcept override { projectionUpdate = true; }

		inline const DirectX::XMFLOAT3& GetPos() const noexcept override { return position; }
		inline DirectX::FXMMATRIX GetProjection() const noexcept override { return DirectX::XMMatrixPerspectiveFovLH(projection.fov, projection.screenRatio, projection.nearClip, projection.farClip); }
		DirectX::FXMMATRIX GetView() const noexcept override { return DirectX::XMLoadFloat4x4(&view); }

		void Roll(float delta) noexcept override;

		void Update(GFX::Graphics& gfx) const noexcept override;
		void Accept(GFX::Graphics& gfx, GFX::Probe::BaseProbe& probe) noexcept override;
		inline void Submit() noexcept override { indicator->Submit(); }
	};
}