#pragma once
#include "GFX/CommandList.h"
#include "GFX/Surface.h"
#include "GFX/ShaderSlot.h"
#include "D3D12.h"

namespace ZE::GFX::API::DX12::Resource
{
	class TextureCube final
	{
	public:
		TextureCube(GFX::Device& dev, const std::array<Surface, 6>& surfaces);
		TextureCube(TextureCube&&) = default;
		TextureCube(const TextureCube&) = delete;
		TextureCube& operator=(TextureCube&&) = default;
		TextureCube& operator=(const TextureCube&) = delete;
		~TextureCube() = default;

		void BindPS(GFX::CommandList& cl, ShaderSlot slot) const noexcept;
		void BindCS(GFX::CommandList& ctx, ShaderSlot slot) const noexcept;
		std::array<Surface, 6> GetData(GFX::Device& dev, GFX::CommandList& cl) const;
	};
}