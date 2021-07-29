#pragma once
#include "D3D12.h"

namespace ZE::GFX::API::DX12
{
	class CommandList final
	{
		DX::ComPtr<ID3D12GraphicsCommandList5> commands;
		DX::ComPtr<ID3D12CommandAllocator> allocator;

	public:
		CommandList() = default;
		CommandList(CommandList&&) = default;
		CommandList(const CommandList&) = delete;
		CommandList& operator=(CommandList&&) = default;
		CommandList& operator=(const CommandList&) = delete;
		~CommandList() = default;

		ID3D12GraphicsCommandList5* GetList() noexcept { return commands.Get(); }
	};
}