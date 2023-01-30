#include "GFX/API/DX/DebugInfoManager.h"
#include "Platform/WinAPI/WinApiException.h"
#include <combaseapi.h>

namespace ZE::GFX::API::DX
{
	DebugInfoManager::DebugInfoManager()
	{
		ZE_WIN_ENABLE_EXCEPT();
		typedef HRESULT(WINAPI* DXGIGetDebugInterface)(REFIID, void**);

		const auto dxgiDebugModule = LoadLibraryExW(L"dxgidebug.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
		if (dxgiDebugModule == nullptr)
			throw ZE_WIN_EXCEPT_LAST();

		const auto dxgiGetDebugInterface = reinterpret_cast<DXGIGetDebugInterface>(reinterpret_cast<void*>(GetProcAddress(dxgiDebugModule, "DXGIGetDebugInterface")));
		if (dxgiGetDebugInterface == nullptr)
			throw ZE_WIN_EXCEPT_LAST();

		ZE_WIN_THROW_FAILED(dxgiGetDebugInterface(IID_PPV_ARGS(&infoQueue)));
		ZE_WIN_THROW_FAILED(dxgiGetDebugInterface(IID_PPV_ARGS(&debug)));
	}

	std::vector<std::string> DebugInfoManager::GetMessages() const
	{
		ZE_WIN_ENABLE_EXCEPT();

		std::vector<std::string> messages;
		const U64 end = infoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL);
		for (U64 i = offset; i < end; ++i)
		{
			SIZE_T msgLen = 0;
			ZE_WIN_THROW_FAILED(infoQueue->GetMessageW(DXGI_DEBUG_ALL, i, nullptr, &msgLen)); // Get length of msg

			std::unique_ptr<U8[]> bytes = std::make_unique<U8[]>(msgLen);
			DXGI_INFO_QUEUE_MESSAGE* msg = reinterpret_cast<DXGI_INFO_QUEUE_MESSAGE*>(bytes.get());
			ZE_WIN_THROW_FAILED(infoQueue->GetMessage(DXGI_DEBUG_ALL, i, msg, &msgLen)); // Get msg

			messages.emplace_back(msg->pDescription);
		}
		return messages;
	}
}