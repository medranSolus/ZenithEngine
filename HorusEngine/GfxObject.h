#pragma once
#include <DirectXMath.h>
#include <memory>

namespace GFX
{
	class GfxObject
	{
	protected:
		mutable std::shared_ptr<DirectX::XMFLOAT4X4> transform = nullptr;

	public:
		inline GfxObject(bool init = true) noexcept { if (init) transform = std::make_shared<DirectX::XMFLOAT4X4>(); }
		inline GfxObject(std::shared_ptr<DirectX::XMFLOAT4X4> transform) : transform(transform) {}
		GfxObject(const GfxObject&) = default;
		GfxObject& operator=(const GfxObject&) = default;
		virtual ~GfxObject() = default;

		inline DirectX::XMMATRIX GetTransformMatrix() const noexcept { return DirectX::XMLoadFloat4x4(transform.get()); }
		inline void SetTransformMatrix(const DirectX::XMFLOAT4X4& transformMatrix) noexcept { transform = std::make_shared<DirectX::XMFLOAT4X4>(transformMatrix); }
		inline void SetTransformMatrix(std::shared_ptr<DirectX::XMFLOAT4X4> transformMatrix) noexcept { transform = transformMatrix; }
	};
}