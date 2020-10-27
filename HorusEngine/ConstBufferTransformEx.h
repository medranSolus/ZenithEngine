#pragma once
#include "ConstBufferTransform.h"

namespace GFX::Resource
{
	class ConstBufferTransformEx : public ConstBufferTransform
	{
		DirectX::XMFLOAT4X4 transform;

	public:
		inline ConstBufferTransformEx(Graphics& gfx, const GfxObject& parent, UINT slot = 0U)
			: ConstBufferTransform(gfx, parent, slot)
		{
			DirectX::XMStoreFloat4x4(&transform, DirectX::XMMatrixIdentity());
		}
		virtual ~ConstBufferTransformEx() = default;

		constexpr void UpdateTransform(const DirectX::XMFLOAT4X4& transformMatrix) noexcept { transform = transformMatrix; }
		inline DirectX::FXMMATRIX GetTransform() const noexcept override { return DirectX::XMLoadFloat4x4(&transform) * ConstBufferTransform::GetTransform(); }

		DirectX::XMFLOAT3 GetPos() const noexcept override;
	};
}