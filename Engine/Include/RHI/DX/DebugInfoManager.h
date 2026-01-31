#pragma once
#include "Platform/WinAPI/ComPtr.h"
ZE_WARNING_PUSH
#include <d3dcommon.h>
#include <dxgidebug.h>
ZE_WARNING_POP

namespace ZE::RHI::DX
{
	// Enable ComPtr for all DX namespace
	using Platform::WinAPI::ComPtr;

	// Wrappers for DXGI interfaces (rest is in DXGI.h)
	typedef IDXGIInfoQueue IInfoQueue;
	typedef IDXGIDebug1    IDebug;

	// Retrieving info from DirectX Debug Layer
	class DebugInfoManager final
	{
		U64 offset = 0;
		ComPtr<IInfoQueue> infoQueue = nullptr;
		ComPtr<IDebug> debug = nullptr;

	public:
		DebugInfoManager();
		ZE_CLASS_MOVE(DebugInfoManager);
		~DebugInfoManager() { infoQueue.Reset(); debug->ReportLiveObjects(DXGI_DEBUG_ALL, static_cast<DXGI_DEBUG_RLO_FLAGS>(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL)); }

		void BeginRecord() noexcept { offset = infoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL); }

		std::vector<std::string> GetMessages() const;
	};
}