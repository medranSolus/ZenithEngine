#pragma once
#include "GFX/Resource/GenericResourceDesc.h"
#include "GFX/CommandList.h"

namespace ZE::RHI::DX12::Resource
{
	class Generic final
	{
		DX::ComPtr<IResource> resource;
		Ptr<U8> buffer;
		DescriptorInfo srvDescriptor;
		DescriptorInfo uavDescriptorGpu;
		DescriptorInfo uavDescriptorCpu;

	public:
		Generic() = default;
		Generic(GFX::Device& dev, const GFX::Resource::GenericResourceDesc& desc);
		ZE_CLASS_MOVE(Generic);
		~Generic() { ZE_ASSERT_FREED(resource == nullptr); }

		constexpr U8* GetBuffer() noexcept { return buffer; }

		void Free(GFX::Device& dev) noexcept;
		bool IsStagingCopyRequired(GFX::Device& dev, const GFX::Resource::GenericResourceDesc& desc) const noexcept;
	};
}