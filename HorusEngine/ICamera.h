#pragma once
#include "IRenderable.h"
#include <DirectXCollision.h>

namespace Camera
{
	class ICamera : public GFX::Pipeline::IRenderable
	{
		std::string name;

	protected:
		mutable bool viewUpdate = true;
		mutable bool projectionUpdate = true;

	public:
		inline ICamera(const std::string& name) noexcept : name(name) {}
		virtual ~ICamera() = default;

		constexpr const std::string& GetName() const noexcept { return name; }
		constexpr void Reset() const noexcept { ResetView(); ResetProjection(); }
		constexpr void ResetView() const noexcept { viewUpdate = true; }
		constexpr void ResetProjection() const noexcept { projectionUpdate = true; }

		virtual void SetPos(const DirectX::XMFLOAT3& pos) noexcept = 0;
		virtual const DirectX::XMFLOAT3& GetPos() const noexcept = 0;
		virtual DirectX::XMMATRIX GetProjection() const noexcept = 0;
		virtual DirectX::XMMATRIX GetView() const noexcept = 0;
		virtual DirectX::BoundingFrustum GetFrustum() const noexcept = 0;

		virtual void MoveX(float dX) noexcept = 0;
		virtual void MoveY(float dY) noexcept = 0;
		virtual void MoveZ(float dZ) noexcept = 0;

		virtual void Rotate(float angleDX, float angleDY) noexcept = 0;
		virtual void Roll(float delta) noexcept = 0;

		virtual void BindCamera(GFX::Graphics& gfx) const noexcept = 0;
		virtual void BindVS(GFX::Graphics& gfx) = 0;
		virtual void BindPS(GFX::Graphics& gfx) = 0;
	};
}