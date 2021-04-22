#pragma once
#include "ICamera.h"
#include "CameraParams.h"
#include "GFX/Shape/CameraFrustum.h"
#include "GFX/Shape/CameraIndicator.h"

namespace Camera
{
	class BaseCamera : public ICamera
	{
	protected:
		static constexpr float MOVE_EPSILON = 0.000001f - FLT_EPSILON;
		static constexpr float FLIP_EPSILON = 16.0f * FLT_EPSILON;

		mutable Float4x4 viewMatrix;
		mutable Float4x4 projectionMatrix;
		ProjectionData projection;
		Float3 up = { 0.0f, 1.0f, 0.0f };
		Float3 moveDirection = { 0.0f, 0.0f, 1.0f };
		bool positionUpdate = true;
		GfxResPtr<GFX::Resource::ConstBufferVertex<Float4>> positionBuffer;
		GfxResPtr<GFX::Resource::ConstBufferExPixelCache> cameraBuffer;
		bool enableIndicator = true;
		bool enableFrustum = false;
		mutable std::unique_ptr<GFX::Shape::CameraIndicator> indicator = nullptr;
		mutable std::unique_ptr<GFX::Shape::CameraFrustum> frustum = nullptr;

		static GFX::Data::CBuffer::DCBLayout MakeLayoutPS() noexcept;

		virtual Matrix UpdateView() const noexcept = 0;
		Matrix UpdateProjection() const noexcept;
		void UpdateBufferVS(GFX::Graphics& gfx);
		void UpdateBufferPS();

	public:
		BaseCamera(GFX::Graphics& gfx, GFX::Pipeline::RenderGraph& graph, CameraParams&& params) noexcept;
		BaseCamera(BaseCamera&&) = default;
		BaseCamera(const BaseCamera&) = default;
		BaseCamera& operator=(BaseCamera&&) = default;
		BaseCamera& operator=(const BaseCamera&) = default;
		virtual ~BaseCamera() = default;

		constexpr void EnableIndicator() noexcept { enableIndicator = true; }
		constexpr void DisableIndicator() noexcept { enableIndicator = false; }
		constexpr void EnableFrustumIndicator() noexcept { enableFrustum = true; }
		constexpr void DisableFrustumIndicator() noexcept { enableFrustum = false; }
		constexpr void BindCamera(GFX::Graphics& gfx) const noexcept override { gfx.SetView(GetView()); gfx.SetProjection(GetProjection()); }

		void SetPos(const Float3& pos) noexcept override { cameraBuffer->GetBuffer()["cameraPos"] = pos; positionUpdate = viewUpdate = true; }
		const Float3& GetPos() const noexcept override { return cameraBuffer->GetBufferConst()["cameraPos"]; }

		void SetOutline() noexcept override { indicator->SetOutline(); }
		void DisableOutline() noexcept override { indicator->DisableOutline(); }

		void BindVS(GFX::Graphics& gfx) override { UpdateBufferVS(gfx); positionBuffer->Bind(gfx); }
		void BindPS(GFX::Graphics& gfx) override { UpdateBufferPS(); cameraBuffer->Bind(gfx); }

		Matrix GetProjection() const noexcept override;
		Matrix GetView() const noexcept override;
		Math::BoundingFrustum GetFrustum() const noexcept override;

		void MoveZ(float dZ) noexcept override;
		void Roll(float delta) noexcept override;
		bool Accept(GFX::Graphics& gfx, GFX::Probe::BaseProbe& probe) noexcept override;
		void Submit(U64 channelFilter) const noexcept override;
	};
}