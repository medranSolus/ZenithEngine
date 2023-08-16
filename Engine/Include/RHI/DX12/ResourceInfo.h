#pragma once
#include "DX12.h"

namespace ZE::RHI::DX12
{
	// Resource information holding reference to created resource
	struct ResourceInfo
	{
		DX::ComPtr<IResource> Resource = nullptr;
		AllocHandle Handle = 0;

		bool IsFree() const noexcept { return Resource == nullptr && Handle == 0; }
	};
}