#pragma once
#include "ImageException.h"
#include "WinApiException.h"

namespace Exception
{
	class DirectXTexException : public ImageException, public WinApiException
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
#define DXT_ENABLE_EXCEPT() HRESULT __hResult
// Before using needs call to DXT_ENABLE_EXCEPT()
#define DXT_EXCEPT(info) Exception::DirectXTexException(__LINE__, __FILE__, __hResult, info)
// Before using needs call to DXT_ENABLE_EXCEPT()
#define	DXT_THROW_FAILED(call, info) if( FAILED(__hResult = (call))) throw DXT_EXCEPT(info)