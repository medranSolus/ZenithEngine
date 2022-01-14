#pragma once
#include "GFX/Device.h"
#include "D3D12.h"

namespace ZE::GFX::API::DX12
{
	class GPerf final
	{
		DX::ComPtr<ID3D12QueryHeap> queryHeap;
		DX::ComPtr<ID3D12Resource> data;
		D3D12_COMMAND_LIST_TYPE listType = D3D12_COMMAND_LIST_TYPE_DIRECT;

	public:
		GPerf(GFX::Device& dev);
		ZE_CLASS_MOVE(GPerf);
		~GPerf() = default;

		static constexpr const char* GetApiString() noexcept { return "DX12"; }

		void Start(GFX::CommandList& cl) noexcept;
		void Stop(GFX::CommandList& cl) const noexcept;
		long double GetData(GFX::Device& dev) noexcept;
	};
}