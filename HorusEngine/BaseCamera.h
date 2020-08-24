#pragma once
#include "ICamera.h"

namespace Camera
{
	class BaseCamera : public ICamera
	{
	protected:
		mutable DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT3 position;
		float fov;
		DirectX::XMFLOAT3 up = { 0.0f, 1.0f, 0.0f };
		float screenRatio;
		float nearClip;
		float farClip;
		mutable bool viewUpdate = true;
		mutable bool projectionUpdate = true;

		virtual DirectX::FXMMATRIX UpdateView() const noexcept = 0;

	public:
		inline BaseCamera(const std::string& name, float fov, float screenRatio, float nearClip, float farClip, const DirectX::XMFLOAT3& position) noexcept
			: ICamera(name), position(position), fov(fov), screenRatio(screenRatio), nearClip(nearClip), farClip(farClip) {}
		BaseCamera(const BaseCamera&) = default;
		BaseCamera& operator=(const BaseCamera&) = default;
		virtual ~BaseCamera() = default;

		inline void ResetView() const noexcept override { viewUpdate = true; }
		inline void ResetProjection() const noexcept override { projectionUpdate = true; }

		inline const DirectX::XMFLOAT3& GetPos() const noexcept override { return position; }
		inline DirectX::FXMMATRIX GetProjection() const noexcept override { return DirectX::XMMatrixPerspectiveFovLH(fov, screenRatio, nearClip, farClip); }
		DirectX::FXMMATRIX GetView() const noexcept override;

		void Roll(float delta) noexcept override;

		void Update(GFX::Graphics& gfx) const noexcept override;
		void ShowWindow() noexcept override;
	};
}