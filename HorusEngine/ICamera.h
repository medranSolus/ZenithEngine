#pragma once
#include "IRenderable.h"

namespace Camera
{
	class ICamera : public GFX::Pipeline::IRenderable
	{
		std::string name;

	public:
		inline ICamera(const std::string& name) noexcept : name(name) {}
		virtual ~ICamera() = default;

		constexpr const std::string& GetName() const noexcept { return name; }
		inline void Reset() const noexcept { ResetView(); ResetProjection(); }

		virtual void ResetView() const noexcept = 0;
		virtual void ResetProjection() const noexcept = 0;

		virtual void SetPos(const DirectX::XMFLOAT3& pos) noexcept = 0;
		virtual const DirectX::XMFLOAT3& GetPos() const noexcept = 0;
		virtual DirectX::FXMMATRIX GetProjection() const noexcept = 0;
		virtual DirectX::FXMMATRIX GetView() const noexcept = 0;

		virtual void MoveX(float dX) noexcept = 0;
		virtual void MoveY(float dY) noexcept = 0;
		virtual void MoveZ(float dZ) noexcept = 0;

		virtual void Rotate(float angleDX, float angleDY) noexcept = 0;
		virtual void Roll(float delta) noexcept = 0;

		virtual void BindCamera(GFX::Graphics& gfx) const noexcept = 0;
		virtual void Bind(GFX::Graphics& gfx) = 0;
	};
}