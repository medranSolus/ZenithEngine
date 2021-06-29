#pragma once
#include "Platform/WinAPI/WinApiException.h"

namespace ZE::GFX::API::DX
{
	class GraphicsException : public WinAPI::WinApiException
	{
#ifdef _ZE_MODE_DEBUG
		std::vector<std::string> debugInfo;

	public:
		GraphicsException(U32 line, const char* file, HRESULT hResult,
			std::vector<std::string>&& info) noexcept
			: BasicException(line, file), WinApiException(line, file, hResult),
			debugInfo(std::move(info)) {}
#else
	public:
		GraphicsException(U32 line, const char* file, HRESULT hResult) noexcept
			: BasicException(line, file), WinApiException(line, file, hResult) {}
#endif
		GraphicsException(GraphicsException&&) = default;
		GraphicsException(const GraphicsException&) = default;
		GraphicsException& operator=(GraphicsException&&) = default;
		GraphicsException& operator=(const GraphicsException&) = default;
		virtual ~GraphicsException() = default;

		constexpr const char* GetType() const noexcept override { return "Graphics Exception"; }

#ifdef _ZE_MODE_DEBUG
		std::string GetDebugInfo() const noexcept;
#endif
		const char* what() const noexcept override;
	};
}

#pragma region Exception macros
// Name of variable holding reference to DebugManager
#define ZE_GFX_EXCEPT_MANAGER debugManager

#ifdef _ZE_MODE_DEBUG
// Enables useage of ZE_GFX_*_INFO macros in current scope
#define ZE_GFX_ENABLE_INFO(device) auto& ZE_GFX_EXCEPT_MANAGER = device.GetInfoManager()

// Before using needs call to ZE_GFX_ENABLE()
// To get accurate debug info first call ZE_GFX_SET_DEBUG_WATCH() to catch only important messages
#define	ZE_GFX_EXCEPT(code) ZE::GFX::API::DX::GraphicsException(__LINE__, __FILENAME__, code, ZE_GFX_EXCEPT_MANAGER.GetMessages())

// Before using needs call to ZE_GFX_ENABLE() or ZE_GFX_ENABLE_INFO()
// Used to get accurate debug info from DirectX Debug Layer
#define ZE_GFX_SET_DEBUG_WATCH() ZE_GFX_EXCEPT_MANAGER.BeginRecord()

// Before using needs call to ZE_GFX_ENABLE() or ZE_GFX_ENABLE_INFO()
// Checks debug layer messages and throws if any appears after call
#define	ZE_GFX_THROW_FAILED_INFO(call) ZE_GFX_SET_DEBUG_WATCH(); (call); { auto msg = ZE_GFX_EXCEPT_MANAGER.GetMessages(); if (msg.size()) throw ZE::GFX::API::DX::GraphicsException(__LINE__, __FILENAME__, S_FALSE, std::move(msg)); }

#else
// Enables useage of ZE_GFX_*_INFO macros in current scope
#define ZE_GFX_ENABLE_INFO(device)

// Before using needs call to ZE_GFX_ENABLE()
// To get accurate debug info first call ZE_GFX_SET_DEBUG_WATCH() to catch only important messages
#define	ZE_GFX_EXCEPT(code) ZE::GFX::API::DX::GraphicsException(__LINE__, __FILENAME__, code)

// Before using needs call to ZE_GFX_ENABLE()
// Used to get accurate debug info from DirectX Debug Layer
#define ZE_GFX_SET_DEBUG_WATCH()

// Before using needs call to ZE_GFX_ENABLE() or ZE_GFX_ENABLE_INFO()
// Checks debug layer messages and throws if any appears after call
#define	ZE_GFX_THROW_FAILED_INFO(call) (call)

#endif
// Enables useage of ZE_GFX_* macros in current scope
#define ZE_GFX_ENABLE(device) ZE_WIN_ENABLE_EXCEPT(); ZE_GFX_ENABLE_INFO(device)

// Before using needs call to ZE_GFX_ENABLE()
// Checks HRESULT returned via function and throws on error
#define	ZE_GFX_THROW_FAILED(call) ZE_GFX_SET_DEBUG_WATCH(); if(FAILED(ZE_WIN_EXCEPT_RESULT = (call))) throw ZE_GFX_EXCEPT(ZE_WIN_EXCEPT_RESULT)
#pragma endregion