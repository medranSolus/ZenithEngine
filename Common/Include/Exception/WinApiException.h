#pragma once
#include "BasicException.h"
#include "WinAPI/WinAPI.h"

namespace Exception
{
	class WinApiException : public virtual BasicException
	{
		HRESULT result;

	public:
		WinApiException(U32 line, const char* file, HRESULT hResult) noexcept
			: BasicException(line, file), result(hResult) {}
		WinApiException(const WinApiException&) = default;
		constexpr WinApiException& operator=(const WinApiException&) = default;
		virtual ~WinApiException() = default;

		static std::string TranslateErrorCode(HRESULT code) noexcept;

		constexpr HRESULT GetErrorCode() const noexcept { return result; }
		std::string GetErrorString() const noexcept { return TranslateErrorCode(result); }
		const char* GetType() const noexcept override { return "WinAPI Exception"; }

		const char* what() const noexcept override;
	};
}

#define	WIN_EXCEPT_LAST() Exception::WinApiException(__LINE__, __FILE__, GetLastError())
#define	WIN_EXCEPT(code) Exception::WinApiException(__LINE__, __FILE__, code)

// Enables useage of WND_THROW_FAILED macro in current scope
#define WIN_ENABLE_EXCEPT() HRESULT __hResult
// Before using needs call to WND_ENABLE_EXCEPT()
#define	WIN_THROW_FAILED(call) if( FAILED(__hResult = (call))) throw WIN_EXCEPT(__hResult)