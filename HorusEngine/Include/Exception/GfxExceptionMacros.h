#pragma once
#include "DeviceRemovedException.h"

// Enables useage of GFX_*_NOINFO macros in current scope
#define GFX_ENABLE_NOINFO() HRESULT __hResult

// No-debug exception
#define	GFX_EXCEPT_NOINFO(code) Exception::GraphicsException(__LINE__, __FILE__, code)

// Checks HRESULT returned via function and throws on error
// Before using needs call to one of GFX_ENABLE_* macros
#define	GFX_THROW_FAILED_NOINFO(call) if(FAILED(__hResult = (call))) throw GFX_EXCEPT_NOINFO(__hResult)

#ifdef _MODE_DEBUG
// DEBUG mode only. Enables retrieval of messages from DirectX Debug Layer
// Imports "DXGIDebugInfoManager debugInfoManager" into current scope from GFX::Graphics object
#define GFX_ENABLE_INFO(gfx) DXGIDebugInfoManager& debugInfoManager = gfx.GetInfoManager()

// Requires "DXGIDebugInfoManager debugInfoManager" available in current scope in DEBUG mode
// To get accurate debug info first call GFX_SET_DEBUG_WATCH() to catch only important messages
// Before using needs call to GFX_ENABLE_ALL() or GFX_ENABLE_INFO()
#define	GFX_EXCEPT(code) Exception::GraphicsException(__LINE__, __FILE__, code, debugInfoManager.GetMessages())

// Requires "DXGIDebugInfoManager debugInfoManager" available in current scope in DEBUG mode
// To get accurate debug info first call GFX_SET_DEBUG_WATCH() to set catch only important messages
// Before using needs call to GFX_ENABLE_ALL() or GFX_ENABLE_INFO()
#define	GFX_DEV_REMOVED_EXCEPT(code) Exception::DeviceRemovedException(__LINE__, __FILE__, code, debugInfoManager.GetMessages())

// Used to get accurate debug info from DirectX Debug Layer
// Requires "DXGIDebugInfoManager debugInfoManager" available in current scope in DEBUG mode
// Before using needs call to GFX_ENABLE_ALL() or GFX_ENABLE_INFO()
#define GFX_SET_DEBUG_WATCH() debugInfoManager.BeginRecord()

// Checks HRESULT returned via function and throws on error
// Requires "DXGIDebugInfoManager debugInfoManager" available in current scope in DEBUG mode
// Before using needs call to GFX_ENABLE_ALL()
#define	GFX_THROW_FAILED(call) GFX_SET_DEBUG_WATCH(); if(FAILED(__hResult = (call))) throw GFX_EXCEPT(__hResult)

// DEBUG mode only. Sets debug watch and checks if messages appeared after call. If yes then throws
// Requires "DXGIDebugInfoManager debugInfoManager" available in current scope in DEBUG mode
// Before using needs call to GFX_ENABLE_ALL() or GFX_ENABLE_INFO()
#define GFX_THROW_FAILED_INFO(call) GFX_SET_DEBUG_WATCH(); (call); {auto msgs = debugInfoManager.GetMessages(); if(msgs.size()) throw Exception::GfxDebugException(__LINE__, __FILE__, std::move(msgs));}
#else
// DEBUG mode only. Enables retrieval of messages from DirectX Debug Layer
// Imports "DXGIDebugInfoManager debugInfoManager" into current scope from GFX::Graphics object
#define GFX_ENABLE_INFO(gfx)

// Requires "DXGIDebugInfoManager debugInfoManager" available in current scope in DEBUG mode
// To get accurate debug info first call GFX_SET_DEBUG_WATCH() to catch only important messages
// Before using needs call to GFX_ENABLE_ALL() or GFX_ENABLE_INFO()
#define	GFX_EXCEPT(code) GFX_EXCEPT_NOINFO(code)

// Requires "DXGIDebugInfoManager debugInfoManager" available in current scope in DEBUG mode
// To get accurate debug info first call GFX_SET_DEBUG_WATCH() to set catch only important messages
// Before using needs call to GFX_ENABLE_ALL() or GFX_ENABLE_INFO()
#define	GFX_DEV_REMOVED_EXCEPT(code) Exception::DeviceRemovedException(__LINE__, __FILE__, code)

// Used to get accurate debug info from DirectX Debug Layer
// Requires "DXGIDebugInfoManager debugInfoManager" available in current scope in DEBUG mode
// Before using needs call to GFX_ENABLE_ALL() or GFX_ENABLE_INFO()
#define GFX_SET_DEBUG_WATCH()

// Checks HRESULT returned via function and throws on error
// Requires "DXGIDebugInfoManager debugInfoManager" available in current scope in DEBUG mode
// Before using needs call to GFX_ENABLE_ALL()
#define	GFX_THROW_FAILED(call) GFX_THROW_FAILED_NOINFO(call)

// DEBUG mode only. Sets debug watch and checks if messages appeared after call. If yes then throws
// Requires "DXGIDebugInfoManager debugInfoManager" available in current scope in DEBUG mode
// Before using needs call to GFX_ENABLE_ALL() or GFX_ENABLE_INFO()
#define GFX_THROW_FAILED_INFO(call) (call)
#endif

// Enables useage of GFX_* macros in current scope
// Imports "DXGIDebugInfoManager debugInfoManager" into current scope from GFX::Graphics object
#define GFX_ENABLE_ALL(gfx) GFX_ENABLE_NOINFO(); GFX_ENABLE_INFO(gfx)