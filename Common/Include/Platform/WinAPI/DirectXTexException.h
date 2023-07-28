#pragma once
#include "Exception/GenericException.h"
#include "WinApiException.h"

namespace ZE::WinAPI
{
	// Exception thrown with DirectXTex specific error while processing images
	class DirectXTexException : public Exception::GenericException, public WinApiException
	{
	public:
		DirectXTexException(U32 line, const char* file, HRESULT hResult, std::string&& note) noexcept
			: BasicException(line, file), GenericException(line, file, std::forward<std::string>(note)),
			WinApiException(line, file, hResult) {}
		ZE_CLASS_DEFAULT(DirectXTexException);
		virtual ~DirectXTexException() = default;

		constexpr const char* GetType() const noexcept override { return "DirectXTex Exception"; }

		const char* what() const noexcept override;
	};
}

// Enables useage of ZE_DXT_ macros in current scope
#define ZE_DXT_ENABLE_EXCEPT() [[maybe_unused]] HRESULT __hResult
// Before using needs call to ZE_DXT_ENABLE_EXCEPT()
#define ZE_DXT_EXCEPT(info) ZE::WinAPI::DirectXTexException(__LINE__, __FILENAME__, __hResult, info)
// Before using needs call to ZE_DXT_ENABLE_EXCEPT()
#define	ZE_DXT_THROW_FAILED(call, info) do { if (FAILED(__hResult = (call))) { ZE_BREAK(); throw ZE_DXT_EXCEPT(info); } } while (false)