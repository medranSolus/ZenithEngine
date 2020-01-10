#pragma once
#include "IBindable.h"
#include "Surface.h"

namespace GFX::Resource
{
	class Texture : public IBindable
	{
		UINT slot;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView;

	public:
		Texture(Graphics & gfx, const Surface & surface, UINT slot = 0U);
		Texture(const Texture&) = delete;
		Texture & operator=(const Texture&) = delete;
		~Texture() = default;

		inline void Bind(Graphics & gfx) noexcept override { GetContext(gfx)->PSSetShaderResources(slot, 1U, textureView.GetAddressOf()); }
	};
}
