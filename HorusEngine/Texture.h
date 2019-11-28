#pragma once
#include "IBindable.h"
#include "Surface.h"

namespace GFX::Resource
{
	class Texture : public IBindable
	{
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView;

	public:
		Texture(Graphics & gfx, const Surface & surface);

		inline void Bind(Graphics & gfx) noexcept override { GetContext(gfx)->PSSetShaderResources(0U, 1U, textureView.GetAddressOf()); }
	};
}
