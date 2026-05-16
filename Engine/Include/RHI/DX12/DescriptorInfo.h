#pragma once
#include "DX12.h"

namespace ZE::RHI::DX12
{
	// Information about allocated descriptor
	struct DescriptorInfo
	{
		D3D12_GPU_DESCRIPTOR_HANDLE GPU = {};
		D3D12_CPU_DESCRIPTOR_HANDLE CPU = {};
		AllocHandle Handle = nullptr;
		bool GpuSide = false;

		DescriptorInfo() = default;
		ZE_CLASS_COPY_ONLY(DescriptorInfo);
		constexpr DescriptorInfo(DescriptorInfo&& desc) noexcept;
		constexpr DescriptorInfo& operator=(DescriptorInfo&& desc) noexcept;
		~DescriptorInfo() = default;
	};

#pragma region Functions
	constexpr DescriptorInfo::DescriptorInfo(DescriptorInfo&& desc) noexcept
		: GPU(desc.GPU), CPU(desc.CPU), Handle(desc.Handle), GpuSide(desc.GpuSide)
	{
		desc.Handle = nullptr;
	}

	constexpr DescriptorInfo& DescriptorInfo::operator=(DescriptorInfo&& desc) noexcept
	{
		GPU = desc.GPU;
		CPU = desc.CPU;
		// Needed for safe deallocation
		std::swap(Handle, desc.Handle);
		std::swap(GpuSide, desc.GpuSide);
		return *this;
	}
#pragma endregion
}