#pragma once
#include "GFX/Resource/GenericResourceDesc.h"
#include "GFX/CommandList.h"

namespace ZE::RHI::VK::Resource
{
	class Generic final
	{
		Ptr<U8> buffer;

	public:
		Generic() = default;
		Generic(GFX::Device& dev, const GFX::Resource::GenericResourceDesc& desc) {}
		ZE_CLASS_MOVE(Generic);
		~Generic() {  }

		constexpr U8* GetBuffer() noexcept { return buffer; }
		constexpr bool IsStagingCopyRequired(GFX::Device& dev, const GFX::Resource::GenericResourceDesc& desc) const noexcept { return true; }
		constexpr void Free(GFX::Device& dev) noexcept {}
	};
}