#pragma once
#include "ComPtr.h"
#include <vector>
#include <string>
#include "WarningGuardOn.h"
#include <dxgidebug.h>
#include "WarningGuardOff.h"

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
		ZE_CLASS_MOVE(DebugInfoManager);
		~DebugInfoManager() { infoQueue.Reset(); debug->ReportLiveObjects(DXGI_DEBUG_ALL, static_cast<DXGI_DEBUG_RLO_FLAGS>(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL)); }

		void BeginRecord() noexcept { offset = infoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL); }

		std::vector<std::string> GetMessages() const;
	};
}