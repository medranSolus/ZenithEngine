#pragma once
#include "BasicException.h"
#include "WinAPI.h"

namespace Exception
{
	class WinApiException : public virtual BasicException
	{
		HRESULT result;

	public:
		WinApiException(unsigned int line, const char* file, HRESULT hResult) noexcept
			: BasicException(line, file), result(hResult) {}
		WinApiException(const WinApiException&) = default;
		constexpr WinApiException& operator=(const WinApiException&) = default;
		virtual ~WinApiException() = default;

		static std::string TranslateErrorCode(HRESULT code) noexcept;

		constexpr HRESULT GetErrorCode() const noexcept { return result; }
		inline std::string GetErrorString() const noexcept { return TranslateErrorCode(result); }
		inline const char* GetType() const noexcept override { return "WinAPI Exception"; }

		const char* what() const noexcept override;
	};
}