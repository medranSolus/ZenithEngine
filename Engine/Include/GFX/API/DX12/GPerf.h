#pragma once
#include "GFX/CommandList.h"

namespace ZE::GFX::API::DX12
{
	class GPerf final
	{
		DX::ComPtr<IQueryHeap> queryHeap;
		DX::ComPtr<IResource> data;
		D3D12_COMMAND_LIST_TYPE listType = D3D12_COMMAND_LIST_TYPE_DIRECT;

	public:
		GPerf() = default;
		GPerf(GFX::Device& dev);
		ZE_CLASS_MOVE(GPerf);
		~GPerf() = default;

		static constexpr const char* GetApiString() noexcept { return "DX12"; }

		void Start(GFX::CommandList& cl) noexcept;
		void Stop(GFX::CommandList& cl) const noexcept;
		long double GetData(GFX::Device& dev) noexcept;
	};
}