#pragma once
#include "GFX/Context.h"
#include "GFX/Surface.h"
#include "GFX/ShaderSlot.h"
#include "D3D11.h"

namespace ZE::GFX::API::DX11::Resource
{
	class Texture final
	{
		DX::ComPtr<ID3D11ShaderResourceView> srv;

	public:
		Texture(GFX::Device& dev, GFX::Context& ctx, const Surface& surface);
		Texture(Texture&&) = delete;
		Texture(const Texture&) = delete;
		Texture& operator=(Texture&&) = delete;
		Texture& operator=(const Texture&) = delete;
		~Texture() = default;

		void BindPS(GFX::Context& ctx, ShaderSlot slot) const noexcept;
		void BindCS(GFX::Context& ctx, ShaderSlot slot) const noexcept;
		Surface GetData(GFX::Device& dev, GFX::Context& ctx) const;
	};
}