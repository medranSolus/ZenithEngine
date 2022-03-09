#include "Platform/WinAPI/WinApiException.h"

namespace ZE::WinAPI
{
	std::string WinApiException::TranslateErrorCode(HRESULT code) noexcept
	{
		// Translation of Windows messages to readable format
		LPTSTR msgBuffer = nullptr;
		DWORD msgLen = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr, static_cast<DWORD>(code), MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), reinterpret_cast<LPTSTR>(&msgBuffer), 0, nullptr);
		if (msgLen == 0)
			return "Unknown error code";
		std::string error = msgBuffer;
		LocalFree(msgBuffer);
		return error;
	}

	const char* WinApiException::what() const noexcept
	{
		std::ostringstream stream;
		stream << BasicException::what()
			<< "\n[Code] 0x" << std::hex << result << std::dec
			<< "\n[Message] " << GetErrorString();
		whatBuffer = stream.str();
		return whatBuffer.c_str();
	}
}