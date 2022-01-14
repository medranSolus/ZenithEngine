#pragma once
#include "GFX/Device.h"
#include "GFX/Binding/Context.h"
#include "GFX/Resource/TexturePackDesc.h"
#include "D3D12.h"

namespace ZE::GFX::API::DX12::Resource
{
	class TexturePack final
	{
		D3D12_GPU_DESCRIPTOR_HANDLE startHandle;
		U32 count;
		DescriptorInfo descInfo;
		ResourceInfo* resources = nullptr;

	public:
		TexturePack(GFX::Device& dev, const GFX::Resource::TexturePackDesc& desc);
		ZE_CLASS_MOVE(TexturePack);
		~TexturePack();

		void Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept;
		std::vector<std::vector<Surface>> GetData(GFX::Device& dev) const;
	};
}