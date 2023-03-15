#pragma once
#include "GFX/Resource/Texture/PackDesc.h"
#include "GFX/Binding/Context.h"
#include "GFX/CommandList.h"

namespace ZE::GFX::API::VK::Resource::Texture
{
	class Pack final
	{
	public:
		Pack() = default;
		Pack(GFX::Device& dev, const GFX::Resource::Texture::PackDesc& desc);
		ZE_CLASS_MOVE(Pack);
		~Pack();

		void Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept;
		void Free(GFX::Device& dev) noexcept;
		std::vector<std::vector<Surface>> GetData(GFX::Device& dev) const;
	};
}