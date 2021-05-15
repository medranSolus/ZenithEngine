#pragma once
#include "WinAPI/WinAPI.h"
#include "Types.h"
#pragma warning(disable:4265)
#include <wrl.h>
#pragma warning(default:4265)
#include <dxgidebug.h>
#include <vector>
#include <string>

namespace ZE::GFX
{
	// Manager to get info from DirectX Debug Layer
	class DXGIDebugInfoManager final
	{
		U64 offset = 0U;
		Microsoft::WRL::ComPtr<IDXGIInfoQueue> infoQueue = nullptr;
		Microsoft::WRL::ComPtr<IDXGIDebug> debug = nullptr;

	public:
		DXGIDebugInfoManager();
		DXGIDebugInfoManager(const DXGIDebugInfoManager&) = delete;
		DXGIDebugInfoManager& operator=(const DXGIDebugInfoManager&) = delete;
		~DXGIDebugInfoManager() { debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_IGNORE_INTERNAL); }

		void BeginRecord() noexcept { offset = infoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL); }

		std::vector<std::string> GetMessages() const;
	};
}