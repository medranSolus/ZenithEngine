#pragma once
#include "Types.h"
#include <memory>

namespace GFX
{
	class GfxObject
	{
	protected:
		mutable std::shared_ptr<Float4x4> transform = nullptr;

	public:
		GfxObject(bool init = true) noexcept { if (init) transform = std::make_shared<Float4x4>(); }
		GfxObject(std::shared_ptr<Float4x4> transform) : transform(std::move(transform)) {}
		GfxObject(GfxObject&&) = default;
		GfxObject(const GfxObject&) = default;
		GfxObject& operator=(GfxObject&&) = default;
		GfxObject& operator=(const GfxObject&) = default;
		virtual ~GfxObject() = default;

		const Float4x4& GetTransform() const noexcept { return *transform; }
		Matrix GetTransformMatrix() const noexcept { return Math::XMLoadFloat4x4(transform.get()); }
		void SetTransformMatrix(const Float4x4& transformMatrix) noexcept { transform = std::make_shared<Float4x4>(transformMatrix); }
		void SetTransformMatrix(std::shared_ptr<Float4x4> transformMatrix) noexcept { transform = std::move(transformMatrix); }
	};
}