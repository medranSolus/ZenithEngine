#pragma once
#include "ConstBufferTransform.h"

namespace GFX::Resource
{
	class ConstBufferTransformEx : public ConstBufferTransform
	{
		DirectX::XMFLOAT4X4 transform;

	protected:
		inline DirectX::FXMMATRIX GetTransformView(Graphics& gfx) noexcept override { return DirectX::XMLoadFloat4x4(&transform) * ConstBufferTransform::GetTransformView(gfx); }

	public:
		inline ConstBufferTransformEx(Graphics& gfx, const GfxObject& parent, UINT slot = 0U)
			: ConstBufferTransform(gfx, parent, slot)
		{
			DirectX::XMStoreFloat4x4(&transform, DirectX::XMMatrixIdentity());
		}

		ConstBufferTransformEx(const ConstBufferTransform&) = delete;
		ConstBufferTransformEx& operator=(const ConstBufferTransformEx&) = delete;
		virtual ~ConstBufferTransformEx() = default;

		void UpdateTransform(const DirectX::XMFLOAT4X4& transformMatrix) noexcept { transform = transformMatrix; }
	};
}