#pragma once
#include <DirectXMath.h>
#include <memory>

namespace GFX
{
	class GfxObject
	{
	protected:
		mutable std::shared_ptr<DirectX::XMFLOAT4X4> transform = nullptr;
		mutable std::shared_ptr<DirectX::XMFLOAT4X4> scaling = nullptr;

	public:
		GfxObject(bool init = true);
		inline GfxObject(std::shared_ptr<DirectX::XMFLOAT4X4> transform, std::shared_ptr<DirectX::XMFLOAT4X4> scaling) : transform(transform), scaling(scaling) {}
		GfxObject(const GfxObject&) = default;
		GfxObject & operator=(const GfxObject&) = default;
		virtual ~GfxObject() = default;

		inline DirectX::XMMATRIX GetTransformMatrix() const noexcept { return DirectX::XMLoadFloat4x4(transform.get()); }
		inline void SetTransformMatrix(const DirectX::XMFLOAT4X4 & transformMatrix) noexcept { transform = std::make_shared< DirectX::XMFLOAT4X4>(transformMatrix); }
		inline void SetTransformMatrix(std::shared_ptr<DirectX::XMFLOAT4X4> transformMatrix) noexcept { transform = transformMatrix; }

		inline DirectX::XMMATRIX GetScalingMatrix() const noexcept { return DirectX::XMLoadFloat4x4(scaling.get()); }
		inline void SetScalingMatrix(const DirectX::XMFLOAT4X4 & scalingMatrix) noexcept { scaling = std::make_shared<DirectX::XMFLOAT4X4>(scalingMatrix); }
		inline void SetScalingMatrix(std::shared_ptr<DirectX::XMFLOAT4X4> scalingMatrix) noexcept { scaling = scalingMatrix; }
	};
}
