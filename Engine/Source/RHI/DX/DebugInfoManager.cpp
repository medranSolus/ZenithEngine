#include "RHI/DX/DebugInfoManager.h"
#include <combaseapi.h>

namespace ZE::RHI::DX
{
	DebugInfoManager::~DebugInfoManager()
	{
		bool present = infoQueue != nullptr;
		infoQueue.Reset();
		if (debug)
			debug->ReportLiveObjects(DXGI_DEBUG_ALL, static_cast<DXGI_DEBUG_RLO_FLAGS>(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
		debug.Reset();
		if (dxgiDebugModule && present)
			FreeLibrary(dxgiDebugModule);
	}

	Expected<DebugInfoManager> DebugInfoManager::Create() noexcept
	{
		typedef HRESULT(WINAPI* DXGIGetDebugInterface)(REFIID, void**);

		DebugInfoManager manager = {};
		manager.dxgiDebugModule = LoadLibraryExW(L"dxgidebug.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
		ZE_WIN_RET_FAILED_LAST_EXPECT(manager.dxgiDebugModule == nullptr);

		DXGIGetDebugInterface dxgiGetDebugInterface = reinterpret_cast<DXGIGetDebugInterface>(reinterpret_cast<void*>(GetProcAddress(manager.dxgiDebugModule, "DXGIGetDebugInterface")));
		ZE_WIN_RET_FAILED_LAST_EXPECT(dxgiGetDebugInterface == nullptr);

		ZE_WIN_RET_FAILED_EXPECT(dxgiGetDebugInterface(IID_PPV_ARGS(&manager.infoQueue)));
		ZE_WIN_RET_FAILED_EXPECT(dxgiGetDebugInterface(IID_PPV_ARGS(&manager.debug)));
		return manager;
	}

	bool DebugInfoManager::CheckMessages() noexcept
	{
		if (instance == nullptr)
			return false;

		const U64 count = instance->infoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL);
		if (count)
		{
			Logger::Error("Found " + std::to_string(count) + " DirectX error messages!");
			std::vector<U8> bytes = {};
			for (U64 i = 0; i < count; ++i)
			{
				SIZE_T msgLen = 0;
				HRESULT hr = instance->infoQueue->GetMessageW(DXGI_DEBUG_ALL, i, nullptr, &msgLen); // Get length of msg
				if (SUCCEEDED(hr))
				{
					bytes.resize(msgLen);
					std::memset(bytes.data(), 0, msgLen);
					DXGI_INFO_QUEUE_MESSAGE* msg = reinterpret_cast<DXGI_INFO_QUEUE_MESSAGE*>(bytes.data());
					hr = instance->infoQueue->GetMessageW(DXGI_DEBUG_ALL, i, msg, &msgLen); // Get msg
					if (SUCCEEDED(hr))
						Logger::Unformatted(true, msg->pDescription, true, true, true);
					else
					{
						ZE_FAIL("Failed to retrieve DX debug message!");
					}
				}
				else
				{
					ZE_FAIL("Failed to retrieve length for DX debug message!");
				}

			}
			instance->infoQueue->ClearStoredMessages(DXGI_DEBUG_ALL);
		}
		return count;
	}
}