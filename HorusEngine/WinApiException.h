#pragma once
#include "WinAPI.h"
#include "BasicException.h"

namespace Exception
{
	class WinApiException : public virtual BasicException
	{
		HRESULT result;

	public:
		WinApiException(unsigned int line, const char * file, HRESULT hResult) noexcept : BasicException(line, file), result(hResult) {}
		WinApiException(const WinApiException &) = default;
		constexpr WinApiException & operator=(const WinApiException &) = default;
		virtual ~WinApiException() = default;

		inline const char * GetType() const noexcept override { return "WinAPI Exception"; }
		constexpr HRESULT GetErrorCode() const noexcept { return result; }
		inline std::string GetErrorString() const noexcept { return TranslateErrorCode(result); }

		const char * what() const noexcept override;
		static std::string TranslateErrorCode(HRESULT code) noexcept;
	};
}