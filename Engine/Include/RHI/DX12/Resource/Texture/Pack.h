#pragma once
#include "GFX/Resource/Texture/PackDesc.h"
#include "GFX/Binding/Context.h"
#include "GFX/CommandList.h"

namespace ZE::RHI::DX12::Resource::Texture
{
	class Pack final
	{
		U32 count;
		DescriptorInfo descInfo;
		Ptr<ResourceInfo> resources;

	public:
		Pack() = default;
		Pack(GFX::Device& dev, const GFX::Resource::Texture::PackDesc& desc);
		ZE_CLASS_MOVE(Pack);
		~Pack();

		void Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept;
		void Free(GFX::Device& dev) noexcept;
		std::vector<std::vector<GFX::Surface>> GetData(GFX::Device& dev) const;
	};
}