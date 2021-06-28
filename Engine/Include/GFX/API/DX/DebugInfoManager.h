#pragma once
#include "ComPtr.h"
#include <vector>
#include <string>
#include <dxgidebug.h>

namespace ZE::GFX::API::DX
{
	// Retrieving info from DirectX Debug Layer
	class DebugInfoManager final
	{
		U64 offset = 0;
		ComPtr<IDXGIInfoQueue> infoQueue = nullptr;
		ComPtr<IDXGIDebug> debug = nullptr;

	public:
		DebugInfoManager();
		DebugInfoManager(DebugInfoManager&&) = delete;
		DebugInfoManager(const DebugInfoManager&) = delete;
		DebugInfoManager& operator=(DebugInfoManager&&) = delete;
		DebugInfoManager& operator=(const DebugInfoManager&) = delete;
		~DebugInfoManager() { debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_IGNORE_INTERNAL); }

		void BeginRecord() noexcept { offset = infoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL); }

		std::vector<std::string> GetMessages() const;
	};
}