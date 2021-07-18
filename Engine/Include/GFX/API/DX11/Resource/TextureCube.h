#pragma once
#include "GFX/Context.h"
#include "GFX/Surface.h"
#include "GFX/ShaderSlot.h"
#include "D3D11.h"

namespace ZE::GFX::API::DX11::Resource
{
	class TextureCube final
	{
		DX::ComPtr<ID3D11ShaderResourceView> srv;

	public:
		TextureCube(GFX::Device& dev, const std::array<Surface, 6>& surfaces);
		TextureCube(TextureCube&&) = delete;
		TextureCube(const TextureCube&) = delete;
		TextureCube& operator=(TextureCube&&) = delete;
		TextureCube& operator=(const TextureCube&) = delete;
		~TextureCube() = default;

		void BindPS(GFX::Context& ctx, ShaderSlot slot) const noexcept;
		void BindCS(GFX::Context& ctx, ShaderSlot slot) const noexcept;
		std::array<Surface, 6> GetData(GFX::Device& dev, GFX::Context& ctx) const;
	};
}