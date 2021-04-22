#pragma once
#include "GFX/Pipeline/IRenderable.h"

namespace Camera
{
	class ICamera : public GFX::Pipeline::IRenderable
	{
	protected:
		std::string name;
		mutable bool viewUpdate = true;
		mutable bool projectionUpdate = true;

	public:
		ICamera(const std::string& name) noexcept : name(name) {}
		ICamera(ICamera&&) = default;
		ICamera(const ICamera&) = default;
		ICamera& operator=(ICamera&&) = default;
		ICamera& operator=(const ICamera&) = default;
		virtual ~ICamera() = default;

		constexpr const std::string& GetName() const noexcept { return name; }
		constexpr void Reset() const noexcept { ResetView(); ResetProjection(); }
		constexpr void ResetView() const noexcept { viewUpdate = true; }
		constexpr void ResetProjection() const noexcept { projectionUpdate = true; }

		virtual void SetPos(const Float3& pos) noexcept = 0;
		virtual const Float3& GetPos() const noexcept = 0;
		virtual Matrix GetProjection() const noexcept = 0;
		virtual Matrix GetView() const noexcept = 0;
		virtual Math::BoundingFrustum GetFrustum() const noexcept = 0;

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