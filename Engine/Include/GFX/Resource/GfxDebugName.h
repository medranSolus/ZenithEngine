#pragma once
#include "Exception/GfxExceptionMacros.h"

// Variable name holding debug name
#define ZE_GFX_DEBUG_ID __debugID

#ifdef _ZE_MODE_DEBUG
// Enables usage of ZE_GFX_SET_* macros when no other ZE_GFX_ENABLE_* macro has been called
#define ZE_GFX_ENABLE_RID(gfx) ZE_GFX_ENABLE_ALL(gfx)

// Sets debug name for GPU object. Uses ZE_GFX_DEBUG_ID variable that must be present (via other ZE_GFX_SET_*ID_* macros)
// Before using needs call to ZE_GFX_ENABLE_RID(), ZE_GFX_ENABLE_ALL() or ZE_GFX_ENABLE_INFO()
#define ZE_GFX_SET_ID_EX(child) if (child) { ZE_GFX_THROW_FAILED(child->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(ZE_GFX_DEBUG_ID.size()), ZE_GFX_DEBUG_ID.c_str())); }

// Creates and sets ZE_GFX_DEBUG_ID variable with given id
#define ZE_GFX_SET_ID_SETUP(id) const std::string ZE_GFX_DEBUG_ID = id
#else
// Enables usage of ZE_GFX_SET_* macros when no other ZE_GFX_ENABLE_* macro has been called
#define ZE_GFX_ENABLE_RID(gfx)

// Sets debug name for GPU object. Uses ZE_GFX_DEBUG_ID variable that must be present (via other ZE_GFX_SET_*ID_* macros)
// Before using needs call to ZE_GFX_ENABLE_RID(), ZE_GFX_ENABLE_ALL() or ZE_GFX_ENABLE_INFO()
#define ZE_GFX_SET_ID_EX(child)

// Imports and sets ZE_GFX_DEBUG_ID variable with given id
#define ZE_GFX_SET_ID_SETUP(id)
#endif

// Sets debug name for GPU object with given id and creates ZE_GFX_DEBUG_ID variable
// Before using needs call to ZE_GFX_ENABLE_ALL() or ZE_GFX_ENABLE_INFO()
#define ZE_GFX_SET_ID(child, id) ZE_GFX_SET_ID_SETUP(id); ZE_GFX_SET_ID_EX(child)

// Sets debug name for GPU object with generated RID and creates ZE_GFX_DEBUG_ID variable
// Before using needs call to ZE_GFX_ENABLE_ALL() or ZE_GFX_ENABLE_INFO()
#define ZE_GFX_SET_RID(child) ZE_GFX_SET_ID(child, GetRID())