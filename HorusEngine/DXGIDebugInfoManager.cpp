#include "DXGIDebugInfoManager.h"
#include "WinApiExceptionMacros.h"
#include "GfxExceptionMacros.h"

namespace GFX
{
	DXGIDebugInfoManager::DXGIDebugInfoManager()
	{
		GFX_ENABLE_EXCEPT();
		typedef HRESULT(WINAPI* DXGIGetDebugInterface)(REFIID, void**);

		const auto dxgiDebugModule = LoadLibraryEx("dxgidebug.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
		if (dxgiDebugModule == nullptr)
			throw WIN_EXCEPT_LAST();

		const auto dxgiGetDebugInterface = reinterpret_cast<DXGIGetDebugInterface>(reinterpret_cast<void*>(GetProcAddress(dxgiDebugModule, "DXGIGetDebugInterface")));
		if (dxgiGetDebugInterface == nullptr)
			throw WIN_EXCEPT_LAST();

		GFX_THROW_FAILED_NOINFO(dxgiGetDebugInterface(__uuidof(IDXGIInfoQueue), &infoQueue));
	}

	std::vector<std::string> DXGIDebugInfoManager::GetMessages() const
	{
		GFX_ENABLE_EXCEPT();

		std::vector<std::string> messages;
		const uint64_t end = infoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL);
		for (uint64_t i = offset; i < end; ++i)
		{
			SIZE_T msgLen = 0;
			GFX_THROW_FAILED_NOINFO(infoQueue->GetMessage(DXGI_DEBUG_ALL, i, nullptr, &msgLen)); // Get length of msg

			std::unique_ptr<char[]> bytes = std::make_unique<char[]>(msgLen);
			DXGI_INFO_QUEUE_MESSAGE* msg = reinterpret_cast<DXGI_INFO_QUEUE_MESSAGE*>(bytes.get());
			GFX_THROW_FAILED_NOINFO(infoQueue->GetMessage(DXGI_DEBUG_ALL, i, msg, &msgLen)); // Get msg

			messages.emplace_back(msg->pDescription);
		}
		return messages;
	}
}