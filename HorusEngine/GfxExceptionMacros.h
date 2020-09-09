#pragma once
#include "Graphics.h"

// Enables useage of GFX_ macros in current scope
#define GFX_ENABLE_EXCEPT() HRESULT __hResult

// Before using needs call to GFX_ENABLE_EXCET()
#define	GFX_THROW_FAILED_NOINFO(call) if(FAILED(__hResult = (call))) throw GFX::Graphics::GraphicsException(__LINE__, __FILE__, __hResult)

#define	GFX_EXCEPT_NOINFO(code) GFX::Graphics::GraphicsException(__LINE__, __FILE__, code)

#ifdef _DEBUG
// Requires "DXGIDebugInfoManager debugInfoManager" available in current scope in DEBUG mode
// Note: Can be obtained from GFX::Graphics via macro GFX_ENABLE_INFO(gfx)
// To get accurate debug info first call GFX_SET_DEBUG_WATCH() to catch only important messages
#define	GFX_EXCEPT(code) GFX::Graphics::GraphicsException(__LINE__, __FILE__, code, debugInfoManager.GetMessages())

// Requires "DXGIDebugInfoManager __debugInfoManager" available in current scope in DEBUG mode
// Note: Can be obtained from GFX::Graphics via macro GFX_ENABLE_INFO(gfx)
// To get accurate debug info first call GFX_SET_DEBUG_WATCH() to set catch only important messages
#define	GFX_DEV_REMOVED_EXCEPT(code) GFX::Graphics::DeviceRemovedException(__LINE__, __FILE__, code, debugInfoManager.GetMessages())

// Requires "DXGIDebugInfoManager debugInfoManager" available in current scope in DEBUG mode
// Note: Can be obtained from GFX::Graphics via macro GFX_ENABLE_INFO(gfx)
// Before using needs call to GFX_ENABLE_EXCET()
#define	GFX_THROW_FAILED(call) debugInfoManager.BeginRecord(); if(FAILED(__hResult = (call))) throw GFX::Graphics::GraphicsException(__LINE__, __FILE__, __hResult, debugInfoManager.GetMessages())

// Imports "DXGIDebugInfoManager debugInfoManager" into current scope from GFX::Graphics object (reference required)
#define GFX_ENABLE_INFO(gfx) DXGIDebugInfoManager& debugInfoManager = gfx.GetInfoManager()

// Requires "DXGIDebugInfoManager debugInfoManager" available in current scope in DEBUG mode
// Note: Can be obtained from GFX::Graphics via macro GFX_ENABLE_INFO(gfx)
// Used to get accurate debug info from DirectX Debug Layer
#define GFX_SET_DEBUG_WATCH() debugInfoManager.BeginRecord()

// Requires "DXGIDebugInfoManager debugInfoManager" available in current scope in DEBUG mode
// Note: Can be obtained from GFX::Graphics via macro GFX_ENABLE_INFO(gfx)
#define GFX_THROW_FAILED_INFO(call) debugInfoManager.BeginRecord(); (call); {auto msgs = debugInfoManager.GetMessages(); if(msgs.size()) throw GFX::Graphics::DebugException(__LINE__, __FILE__, msgs);}
#else
// Requires "DXGIDebugInfoManager debugInfoManager" available in current scope in DEBUG mode
// Note: Can be obtained from GFX::Graphics via macro GFX_ENABLE_INFO(gfx)
// To get accurate debug info first call GFX_SET_DEBUG_WATCH() to catch only important messages
#define	GFX_EXCEPT(code) GFX_EXCEPT_NOINFO(code)

// Requires "DXGIDebugInfoManager debugInfoManager" available in current scope in DEBUG mode
// Note: Can be obtained from GFX::Graphics via macro GFX_ENABLE_INFO(gfx)
// To get accurate debug info first call GFX_SET_DEBUG_WATCH() to set catch only important messages
#define	GFX_DEV_REMOVED_EXCEPT(code) GFX::Graphics::DeviceRemovedException(__LINE__, __FILE__, code)

// Requires "DXGIDebugInfoManager debugInfoManager" available in current scope in DEBUG mode
// Before using needs call to GFX_ENABLE_EXCET()
#define	GFX_THROW_FAILED(call) GFX_THROW_FAILED_NOINFO(call)

// Imports "DXGIDebugInfoManager debugInfoManager" into current scope from GFX::Graphics object (reference required)
#define GFX_ENABLE_INFO(gfx)

// Requires "DXGIDebugInfoManager debugInfoManager" available in current scope in DEBUG mode
// Note: Can be obtained from GFX::Graphics via macro GFX_ENABLE_INFO(gfx)
// Used to get accurate debug info from DirectX Debug Layer
#define GFX_SET_DEBUG_WATCH()

// Requires "DXGIDebugInfoManager debugInfoManager" available in current scope in DEBUG mode
// Note: Can be obtained from GFX::Graphics via macro GFX_ENABLE_INFO(gfx)
#define GFX_THROW_FAILED_INFO(call) (call)
#endif

// Enables useage of GFX_ macros in current scope
// Imports "DXGIDebugInfoManager debugInfoManager" into current scope from GFX::Graphics object (reference required)
#define GFX_ENABLE_ALL(gfx) GFX_ENABLE_EXCEPT(); GFX_ENABLE_INFO(gfx)