#pragma once
#include "D3D12.h"

namespace ZE::GFX::API::DX12
{
	// Identifier to single allocation
	typedef void* AllocHandle;

	// Resource information holding reference to created resource
	struct ResourceInfo
	{
		DX::ComPtr<ID3D12Resource> Resource = nullptr;
		AllocHandle Handle = 0;
	};
}