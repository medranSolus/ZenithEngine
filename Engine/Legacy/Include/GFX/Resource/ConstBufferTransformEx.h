#pragma once
#include "ConstBufferTransform.h"

namespace ZE::GFX::Resource
{
	class ConstBufferTransformEx : public ConstBufferTransform
	{
		Float4x4 transform;

	public:
		ConstBufferTransformEx(Graphics& gfx, const GfxObject& parent, U32 slot = 0);
		virtual ~ConstBufferTransformEx() = default;

		constexpr void UpdateTransform(const Float4x4& transformMatrix) noexcept { transform = transformMatrix; }
		constexpr void UpdateTransform(Float4x4&& transformMatrix) noexcept { transform = std::move(transformMatrix); }
		Matrix GetTransform() const noexcept override { return Math::XMLoadFloat4x4(&transform) * ConstBufferTransform::GetTransform(); }

		Float3 GetPos() const noexcept override;
	};
}