#pragma once
#include "GFX/Resource/Texture/PackDesc.h"
#include "GFX/Binding/Context.h"
#include "GFX/CommandList.h"
#include "GFX/GFile.h"

namespace ZE::RHI::DX12::Resource::Texture
{
	class Pack final
	{
		static_assert(Math::AlignUp(GFX::Surface::ROW_PITCH_ALIGNMENT, static_cast<U32>(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT)) == GFX::Surface::ROW_PITCH_ALIGNMENT,
			"For platform supporting DX12 texture rows' alignment must be aligned to D3D12_TEXTURE_DATA_PITCH_ALIGNMENT!");
		static_assert(Math::AlignUp(GFX::Surface::SLICE_PITCH_ALIGNMENT, static_cast<U64>(D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT)) == GFX::Surface::SLICE_PITCH_ALIGNMENT,
			"For platform supporting DX12 texture slice's alignment must be aligned to D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT!");

		U32 count;
		DescriptorInfo descInfo;
		Ptr<ResourceInfo> resources;

	public:
		Pack() = default;
		Pack(GFX::Device& dev, GFX::DiskManager& disk, const GFX::Resource::Texture::PackDesc& desc);
		Pack(GFX::Device& dev, GFX::DiskManager& disk, const GFX::Resource::Texture::PackFileDesc& desc, GFX::GFile& file);
		ZE_CLASS_MOVE(Pack);
		~Pack();

		void Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept;
		void Free(GFX::Device& dev) noexcept;
		std::vector<std::vector<GFX::Surface>> GetData(GFX::Device& dev) const;

		// Gfx API Internal

		constexpr U32 GetTextureCount() const noexcept { return count; }
		constexpr const DescriptorInfo& GetDescInfo() const noexcept { return descInfo; }
		IResource* GetResource(U32 index) const noexcept { ZE_ASSERT(index < count, "Texture resource index out of range!"); return resources[index].Resource.Get(); }
		DX::ComPtr<IResource> GetRes(U32 index) const noexcept { ZE_ASSERT(index < count, "Texture resource index out of range!"); return resources[index].Resource; }
	};
}