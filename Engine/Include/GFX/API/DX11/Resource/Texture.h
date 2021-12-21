#pragma once
#include "GFX/CommandList.h"
#include "GFX/Surface.h"
#include "GFX/ShaderSlot.h"
#include "D3D11.h"

namespace ZE::GFX::API::DX11::Resource
{
	class Texture final
	{
		DX::ComPtr<ID3D11ShaderResourceView> srv;

	public:
		Texture(GFX::Device& dev, GFX::CommandList& cl, const Surface& surface);
		ZE_CLASS_MOVE(Texture);
		~Texture() = default;

		void BindPS(GFX::CommandList& cl, ShaderSlot slot) const noexcept;
		void BindCS(GFX::CommandList& cl, ShaderSlot slot) const noexcept;
		Surface GetData(GFX::Device& dev, GFX::CommandList& cl) const;
	};
}