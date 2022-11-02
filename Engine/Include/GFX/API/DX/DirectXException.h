#pragma once
#include "Platform/WinAPI/WinApiException.h"

namespace ZE::GFX::API::DX
{
	// Standard exception for DirectX Graphics Infrastructure based APIs
	class DirectXException : public WinAPI::WinApiException
	{
#if _ZE_DEBUG_GFX_API
		std::vector<std::string> debugInfo;

	public:
		DirectXException(U32 line, const char* file, HRESULT hResult,
			std::vector<std::string>&& info) noexcept
			: BasicException(line, file), WinApiException(line, file, hResult),
			debugInfo(std::move(info)) {}
#else
	public:
		DirectXException(U32 line, const char* file, HRESULT hResult) noexcept
			: BasicException(line, file), WinApiException(line, file, hResult) {}
#endif
		ZE_CLASS_DEFAULT(DirectXException);
		virtual ~DirectXException() = default;

		constexpr const char* GetType() const noexcept override { return "DirectX Graphics Exception"; }

#if _ZE_DEBUG_GFX_API
		std::string GetDebugInfo() const noexcept;
#endif
		const char* what() const noexcept override;
	};
}

#pragma region Exception macros
// Name of variable holding reference to DebugManager
#define ZE_DX_EXCEPT_MANAGER debugManager

#if _ZE_DEBUG_GFX_API
// Enables useage of ZE_DX_*_INFO macros in current scope
#define ZE_DX_ENABLE_INFO(device) auto& ZE_DX_EXCEPT_MANAGER = device.GetInfoManager()

// Before using needs call to ZE_DX_ENABLE()
// To get accurate debug info first call ZE_DX_SET_DEBUG_WATCH() to catch only important messages
#define	ZE_DX_EXCEPT(code) ZE::GFX::API::DX::DirectXException(__LINE__, __FILENAME__, code, ZE_DX_EXCEPT_MANAGER.GetMessages())

// Before using needs call to ZE_DX_ENABLE() or ZE_DX_ENABLE_INFO()
// Used to get accurate debug info from DirectX Debug Layer
#define ZE_DX_SET_DEBUG_WATCH() ZE_DX_EXCEPT_MANAGER.BeginRecord()

// Before using needs call to ZE_DX_ENABLE() or ZE_DX_ENABLE_INFO()
// Checks debug layer messages and throws if any appears after call
#define	ZE_DX_THROW_FAILED_INFO(call) ZE_DX_SET_DEBUG_WATCH(); (call); { auto msg = ZE_DX_EXCEPT_MANAGER.GetMessages(); if (msg.size()) throw ZE::GFX::API::DX::DirectXException(__LINE__, __FILENAME__, S_FALSE, std::move(msg)); }

#else
// Enables useage of ZE_DX_*_INFO macros in current scope
#define ZE_DX_ENABLE_INFO(device)

// Before using needs call to ZE_DX_ENABLE()
// To get accurate debug info first call ZE_DX_SET_DEBUG_WATCH() to catch only important messages
#define	ZE_DX_EXCEPT(code) ZE::GFX::API::DX::DirectXException(__LINE__, __FILENAME__, code)

// Before using needs call to ZE_DX_ENABLE()
// Used to get accurate debug info from DirectX Debug Layer
#define ZE_DX_SET_DEBUG_WATCH()

// Before using needs call to ZE_DX_ENABLE() or ZE_DX_ENABLE_INFO()
// Checks debug layer messages and throws if any appears after call
#define	ZE_DX_THROW_FAILED_INFO(call) (call)

#endif
// Enables useage of ZE_DX_* macros in current scope
#define ZE_DX_ENABLE(device) ZE_WIN_ENABLE_EXCEPT(); ZE_DX_ENABLE_INFO(device)

// Before using needs call to ZE_DX_ENABLE()
// Checks HRESULT returned via function and throws on error
#define	ZE_DX_THROW_FAILED_NOINFO(call) if(FAILED(ZE_WIN_EXCEPT_RESULT = (call))) throw ZE_DX_EXCEPT(ZE_WIN_EXCEPT_RESULT)

// Before using needs call to ZE_DX_ENABLE()
// Checks HRESULT returned via function and throws on error
#define	ZE_DX_THROW_FAILED(call) ZE_DX_SET_DEBUG_WATCH(); ZE_DX_THROW_FAILED_NOINFO(call)
#pragma endregion

#pragma region Debug name macros
// Variable name holding debug name
#define ZE_DX_DEBUG_ID __debugID

#if _ZE_DEBUG_GFX_NAMES
// Enables useage of ZE_DX_SET_ID macros in current scope
#define ZE_DX_ENABLE_ID(device) ZE_DX_ENABLE(device); std::string ZE_DX_DEBUG_ID

// Before using needs call to ZE_DX_ENABLE_ID()
// Sets debug name for GPU object with given id
#define ZE_DX_SET_ID(child, id) ZE_DX_DEBUG_ID = id; ZE_DX_THROW_FAILED(child->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(ZE_DX_DEBUG_ID.size()), ZE_DX_DEBUG_ID.c_str()))

#else
// Enables useage of ZE_DX_SET_ID macros in current scope
#define ZE_DX_ENABLE_ID(device) ZE_DX_ENABLE(device)

// Before using needs call to ZE_DX_ENABLE_ID()
// Sets debug name for GPU object with given id
#define ZE_DX_SET_ID(child, id)
#endif
#pragma endregion