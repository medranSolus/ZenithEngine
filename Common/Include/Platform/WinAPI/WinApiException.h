#pragma once
#include "Exception/BasicException.h"
#include "WinAPI.h"

namespace ZE::WinAPI
{
	// Exception thrown related to Windows based errors
	class WinApiException : public virtual Exception::BasicException
	{
		HRESULT result;

	public:
		WinApiException(U32 line, const char* file, HRESULT hResult) noexcept
			: BasicException(line, file), result(hResult) {}
		ZE_CLASS_DEFAULT(WinApiException);
		virtual ~WinApiException() = default;

		static std::string TranslateErrorCode(HRESULT code) noexcept;

		constexpr HRESULT GetErrorCode() const noexcept { return result; }
		constexpr const char* GetType() const noexcept override { return "WinAPI Exception"; }
		std::string GetErrorString() const noexcept { return TranslateErrorCode(result); }

		const char* what() const noexcept override;
	};
}

#define	ZE_WIN_EXCEPT(code) ZE::WinAPI::WinApiException(__LINE__, __FILENAME__, code)
#define	ZE_WIN_EXCEPT_LAST() ZE_WIN_EXCEPT(GetLastError())

// Variable holding result of last Windows call
#define ZE_WIN_EXCEPT_RESULT __hResult
// Enables useage of ZE_WIN_THROW_FAILED macro in current scope
#define ZE_WIN_ENABLE_EXCEPT() HRESULT ZE_WIN_EXCEPT_RESULT
// Before using needs call to WND_ENABLE_EXCEPT()
#define	ZE_WIN_THROW_FAILED(call) if(FAILED(ZE_WIN_EXCEPT_RESULT = (call))) throw ZE_WIN_EXCEPT(ZE_WIN_EXCEPT_RESULT)