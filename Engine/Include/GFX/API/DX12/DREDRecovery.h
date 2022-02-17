#pragma once
#include "D3D12.h"

namespace ZE::GFX::API::DX12
{
	// Device Removed Extended Data handler
	class DREDRecovery
	{
	public:
		struct Data
		{
			U64 PageFaultAddress;
			std::string AutoBreadcrumbs;
			std::string ExistingAllocations;
			std::string FreedAllocations;
		};

	private:
		static constexpr const char* DecodeLastOperation(D3D12_AUTO_BREADCRUMB_OP operation) noexcept;
		static constexpr const char* DecodeAllocation(D3D12_DRED_ALLOCATION_TYPE allocation) noexcept;

	public:
		DREDRecovery() = delete;

		// Must be called before creation of the ID3D12Device
		static void Enable(DX::DebugInfoManager& debugManager);
		// Gather information after receiving device removed
		static void GetDeviceRemovedData(class Device& dev, Data& data);
	};
}