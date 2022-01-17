#pragma once
#include "GFX/Device.h"
#include "GFX/Binding/Context.h"
#include "GFX/Resource/Texture/PackDesc.h"
#include "D3D12.h"

namespace ZE::GFX::API::DX12::Resource::Texture
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
		std::vector<std::vector<Surface>> GetData(GFX::Device& dev) const;
	};
}