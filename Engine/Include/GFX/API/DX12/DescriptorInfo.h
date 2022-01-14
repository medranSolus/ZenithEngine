#pragma once
#include "D3D12.h"

namespace ZE::GFX::API::DX12
{
	// Information about allocated descriptor
	struct DescriptorInfo
	{
		D3D12_GPU_DESCRIPTOR_HANDLE GPU;
		D3D12_CPU_DESCRIPTOR_HANDLE CPU;
		U32 ID;
	};
}