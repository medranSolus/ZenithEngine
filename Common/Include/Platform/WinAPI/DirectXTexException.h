#pragma once
#include "Exception/ImageException.h"
#include "WinApiException.h"

namespace ZE::WinAPI
{
	class DirectXTexException : public Exception::ImageException, public WinApiException
	{
	public:
		DirectXTexException(U32 line, const char* file, HRESULT hResult, std::string&& note) noexcept
			: BasicException(line, file), ImageException(line, file, std::forward<std::string>(note)),
			WinApiException(line, file, hResult) {}
		DirectXTexException(DirectXTexException&&) = default;
		DirectXTexException(const DirectXTexException&) = default;
		DirectXTexException& operator=(DirectXTexException&&) = default;
		DirectXTexException& operator=(const DirectXTexException&) = default;
		virtual ~DirectXTexException() = default;

		constexpr const char* GetType() const noexcept override { return "DirectXTex Exception"; }

		const char* what() const noexcept override;
	};
}

// Enables useage of DXT_ macros in current scope
#define ZE_DXT_ENABLE_EXCEPT() HRESULT __hResult
// Before using needs call to DXT_ENABLE_EXCEPT()
#define ZE_DXT_EXCEPT(info) ZE::WinAPI::DirectXTexException(__LINE__, __FILENAME__, __hResult, info)
// Before using needs call to DXT_ENABLE_EXCEPT()
#define	ZE_DXT_THROW_FAILED(call, info) if( FAILED(__hResult = (call))) throw ZE_DXT_EXCEPT(info)