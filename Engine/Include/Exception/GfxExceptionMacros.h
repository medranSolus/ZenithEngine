#pragma once
#include "DeviceRemovedException.h"

// Enables useage of ZE_GFX_*_NOINFO macros in current scope
#define ZE_GFX_ENABLE_NOINFO() HRESULT __hResult

// No-debug exception
#define	ZE_GFX_EXCEPT_NOINFO(code) ZE::Exception::GraphicsException(__LINE__, __FILE__, code)

// Checks HRESULT returned via function and throws on error
// Before using needs call to one of ZE_GFX_ENABLE_* macros
#define	ZE_GFX_THROW_FAILED_NOINFO(call) if(FAILED(__hResult = (call))) throw ZE_GFX_EXCEPT_NOINFO(__hResult)

#ifdef _ZE_MODE_DEBUG
// DEBUG mode only. Enables retrieval of messages from DirectX Debug Layer
// Imports "ZE::GFX::DXGIDebugInfoManager debugInfoManager" into current scope from ZE::GFX::Graphics object
#define ZE_GFX_ENABLE_INFO(gfx) auto& debugInfoManager = gfx.GetInfoManager()

// Requires "ZE::GFX::DXGIDebugInfoManager debugInfoManager" available in current scope in DEBUG mode
// To get accurate debug info first call ZE_GFX_SET_DEBUG_WATCH() to catch only important messages
// Before using needs call to ZE_GFX_ENABLE_ALL() or ZE_GFX_ENABLE_INFO()
#define	ZE_GFX_EXCEPT(code) ZE::Exception::GraphicsException(__LINE__, __FILE__, code, debugInfoManager.GetMessages())

// Requires "ZE::GFX::DXGIDebugInfoManager debugInfoManager" available in current scope in DEBUG mode
// To get accurate debug info first call ZE_GFX_SET_DEBUG_WATCH() to set catch only important messages
// Before using needs call to ZE_GFX_ENABLE_ALL() or ZE_GFX_ENABLE_INFO()
#define	ZE_GFX_DEV_REMOVED_EXCEPT(code) ZE::Exception::DeviceRemovedException(__LINE__, __FILE__, code, debugInfoManager.GetMessages())

// Used to get accurate debug info from DirectX Debug Layer
// Requires "ZE::GFX::DXGIDebugInfoManager debugInfoManager" available in current scope in DEBUG mode
// Before using needs call to ZE_GFX_ENABLE_ALL() or ZE_GFX_ENABLE_INFO()
#define ZE_GFX_SET_DEBUG_WATCH() debugInfoManager.BeginRecord()

// Checks HRESULT returned via function and throws on error
// Requires "ZE::GFX::DXGIDebugInfoManager debugInfoManager" available in current scope in DEBUG mode
// Before using needs call to ZE_GFX_ENABLE_ALL()
#define	ZE_GFX_THROW_FAILED(call) ZE_GFX_SET_DEBUG_WATCH(); if(FAILED(__hResult = (call))) throw ZE_GFX_EXCEPT(__hResult)

// DEBUG mode only. Sets debug watch and checks if messages appeared after call. If yes then throws
// Requires "ZE::GFX::DXGIDebugInfoManager debugInfoManager" available in current scope in DEBUG mode
// Before using needs call to ZE_GFX_ENABLE_ALL() or ZE_GFX_ENABLE_INFO()
#define ZE_GFX_THROW_FAILED_INFO(call) ZE_GFX_SET_DEBUG_WATCH(); (call); {auto msgs = debugInfoManager.GetMessages(); if(msgs.size()) throw ZE::Exception::GfxDebugException(__LINE__, __FILE__, std::move(msgs));}
#else
// DEBUG mode only. Enables retrieval of messages from DirectX Debug Layer
// Imports "ZE::GFX::DXGIDebugInfoManager debugInfoManager" into current scope from ZE::GFX::Graphics object
#define ZE_GFX_ENABLE_INFO(gfx)

// Requires "ZE::GFX::DXGIDebugInfoManager debugInfoManager" available in current scope in DEBUG mode
// To get accurate debug info first call ZE_GFX_SET_DEBUG_WATCH() to catch only important messages
// Before using needs call to ZE_GFX_ENABLE_ALL() or ZE_GFX_ENABLE_INFO()
#define	ZE_GFX_EXCEPT(code) ZE_GFX_EXCEPT_NOINFO(code)

// Requires "ZE::GFX::DXGIDebugInfoManager debugInfoManager" available in current scope in DEBUG mode
// To get accurate debug info first call ZE_GFX_SET_DEBUG_WATCH() to set catch only important messages
// Before using needs call to ZE_GFX_ENABLE_ALL() or ZE_GFX_ENABLE_INFO()
#define	ZE_GFX_DEV_REMOVED_EXCEPT(code) ZE::Exception::DeviceRemovedException(__LINE__, __FILE__, code)

// Used to get accurate debug info from DirectX Debug Layer
// Requires "ZE::GFX::DXGIDebugInfoManager debugInfoManager" available in current scope in DEBUG mode
// Before using needs call to ZE_GFX_ENABLE_ALL() or ZE_GFX_ENABLE_INFO()
#define ZE_GFX_SET_DEBUG_WATCH()

// Checks HRESULT returned via function and throws on error
// Requires "ZE::GFX::DXGIDebugInfoManager debugInfoManager" available in current scope in DEBUG mode
// Before using needs call to ZE_GFX_ENABLE_ALL()
#define	ZE_GFX_THROW_FAILED(call) ZE_GFX_THROW_FAILED_NOINFO(call)

// DEBUG mode only. Sets debug watch and checks if messages appeared after call. If yes then throws
// Requires "ZE::GFX::DXGIDebugInfoManager debugInfoManager" available in current scope in DEBUG mode
// Before using needs call to ZE_GFX_ENABLE_ALL() or ZE_GFX_ENABLE_INFO()
#define ZE_GFX_THROW_FAILED_INFO(call) (call)
#endif

// Enables useage of ZE_GFX_* macros in current scope
// Imports "ZE::GFX::DXGIDebugInfoManager debugInfoManager" into current scope from ZE::GFX::Graphics object
#define ZE_GFX_ENABLE_ALL(gfx) ZE_GFX_ENABLE_NOINFO(); ZE_GFX_ENABLE_INFO(gfx)