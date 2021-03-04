#pragma once
#include "ICamera.h"
#include "CameraFrustum.h"
#include "CameraIndicator.h"
#include "CameraParams.h"

namespace Camera
{
	class BaseCamera : public ICamera
	{
	protected:
		static constexpr float MOVE_EPSILON = 0.000001f - FLT_EPSILON;
		static constexpr float FLIP_EPSILON = 16.0f * FLT_EPSILON;

		mutable DirectX::XMFLOAT4X4 viewMatrix;
		mutable DirectX::XMFLOAT4X4 projectionMatrix;
		ProjectionData projection;
		DirectX::XMFLOAT3 up = { 0.0f, 1.0f, 0.0f };
		DirectX::XMFLOAT3 moveDirection = { 0.0f, 0.0f, 1.0f };
		bool positionUpdate = true;
		GfxResPtr<GFX::Resource::ConstBufferVertex<DirectX::XMFLOAT4>> positionBuffer;
		GfxResPtr<GFX::Resource::ConstBufferExPixelCache> cameraBuffer;
		bool enableIndicator = true;
		bool enableFrustum = false;
		std::unique_ptr<GFX::Shape::CameraIndicator> indicator = nullptr;
		std::unique_ptr<GFX::Shape::CameraFrustum> frustum = nullptr;

		static inline GFX::Data::CBuffer::DCBLayout MakeLayoutPS() noexcept;

		virtual DirectX::XMMATRIX UpdateView() const noexcept = 0;
		DirectX::XMMATRIX UpdateProjection() const noexcept;
		void UpdateBufferVS(GFX::Graphics& gfx) noexcept;
		void UpdateBufferPS() noexcept;

	public:
		BaseCamera(GFX::Graphics& gfx, GFX::Pipeline::RenderGraph& graph, const CameraParams& params) noexcept;
		BaseCamera(const BaseCamera&) = default;
		BaseCamera& operator=(const BaseCamera&) = default;
		virtual ~BaseCamera() = default;

		constexpr void EnableIndicator() noexcept { enableIndicator = true; }
		constexpr void DisableIndicator() noexcept { enableIndicator = false; }
		constexpr void EnableFrustumIndicator() noexcept { enableFrustum = true; }
		constexpr void DisableFrustumIndicator() noexcept { enableFrustum = false; }

		inline void SetPos(const DirectX::XMFLOAT3& pos) noexcept override { cameraBuffer->GetBuffer()["cameraPos"] = pos; positionUpdate = viewUpdate = true; }
		inline const DirectX::XMFLOAT3& GetPos() const noexcept override { return cameraBuffer->GetBufferConst()["cameraPos"]; }

		inline void SetOutline() noexcept override { indicator->SetOutline(); }
		inline void DisableOutline() noexcept override { indicator->DisableOutline(); }

		inline void BindVS(GFX::Graphics& gfx) override { UpdateBufferVS(gfx); positionBuffer->Bind(gfx); }
		inline void BindPS(GFX::Graphics& gfx) override { UpdateBufferPS(); cameraBuffer->Bind(gfx); }

		DirectX::XMMATRIX GetProjection() const noexcept override;
		DirectX::XMMATRIX GetView() const noexcept override;
		DirectX::BoundingFrustum GetFrustum() const noexcept override;

		void MoveZ(float dZ) noexcept override;
		void Roll(float delta) noexcept override;
		void BindCamera(GFX::Graphics& gfx) const noexcept override;
		bool Accept(GFX::Graphics& gfx, GFX::Probe::BaseProbe& probe) noexcept override;
		void Submit(uint64_t channelFilter) noexcept override;
	};
}