#pragma once
#include "GFX/CommandList.h"
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
		ZE_CLASS_MOVE(TextureCube);
		~TextureCube() = default;

		void BindPS(GFX::CommandList& cl, ShaderSlot slot) const noexcept;
		void BindCS(GFX::CommandList& cl, ShaderSlot slot) const noexcept;
		std::array<Surface, 6> GetData(GFX::Device& dev, GFX::CommandList& cl) const;
	};
}