#pragma once
#include "GFX/Pipeline/FrameBuffer.h"
#include "GFX/Resource/GenericResourceDesc.h"

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
		Generic(GFX::Pipeline::FrameBuffer& framebuffer, RID rid) noexcept;
		ZE_CLASS_MOVE(Generic);
		~Generic() { ZE_ASSERT_FREED(resource == nullptr); }

		constexpr U8* GetBuffer() noexcept { return buffer; }

		bool IsStagingCopyRequired(GFX::Device& dev, const GFX::Resource::GenericResourceDesc& desc) const noexcept;
		void Free(GFX::Device& dev) noexcept;

		// Gfx API Internal

		IResource* GetResource() const noexcept { return resource.Get(); }
	};
}