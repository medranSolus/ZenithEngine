#pragma once
#include "D3D12.h"

namespace ZE::GFX::API::DX12
{
	// Device Removed Extended Data handler
	class DREDRecovery
	{
		static constexpr const char* DecodeLastOperation(D3D12_AUTO_BREADCRUMB_OP operation) noexcept;
		static constexpr const char* DecodeAllocation(D3D12_DRED_ALLOCATION_TYPE allocation) noexcept;
		static constexpr const char* DecodeDxgiError(HRESULT error) noexcept;

	public:
		DREDRecovery() = delete;

		// Must be called before creation of the ID3D12Device
		static void Enable(DX::DebugInfoManager& debugManager);
		// Gather information after receiving device removed
		static void SaveDeviceRemovedData(class Device& dev, const std::string& filename);
	};
}