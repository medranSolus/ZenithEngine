#pragma once
#include "D3D12.h"

namespace ZE::GFX::API::DX12
{
	// Resource information holding reference to created resource
	struct ResourceInfo
	{
		DX::ComPtr<IResource> Resource = nullptr;
		AllocHandle Handle = 0;
	};
}