#pragma once
#include "Exception/GfxExceptionMacros.h"

// Variable name holding debug name
#define GFX_DEBUG_ID __debugID

#ifdef _MODE_DEBUG
// Enables usage of GFX_SET_* macros when no other GFX_ENABLE_* macro has been called
#define GFX_ENABLE_RID(gfx) GFX_ENABLE_ALL(gfx)

// Sets debug name for GPU object. Uses GFX_DEBUG_ID variable that must be present (via other GFX_SET_*ID_* macros)
// Before using needs call to GFX_ENABLE_RID(), GFX_ENABLE_ALL() or GFX_ENABLE_INFO()
#define GFX_SET_ID_EX(child) if (child) { GFX_THROW_FAILED(child->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(GFX_DEBUG_ID.size()), GFX_DEBUG_ID.c_str())); }

// Creates and sets GFX_DEBUG_ID variable with given id
#define GFX_SET_ID_SETUP(id) const std::string GFX_DEBUG_ID = id
#else
// Enables usage of GFX_SET_* macros when no other GFX_ENABLE_* macro has been called
#define GFX_ENABLE_RID(gfx)

// Sets debug name for GPU object. Uses GFX_DEBUG_ID variable that must be present (via other GFX_SET_*ID_* macros)
// Before using needs call to GFX_ENABLE_RID(), GFX_ENABLE_ALL() or GFX_ENABLE_INFO()
#define GFX_SET_ID_EX(child)

// Imports and sets GFX_DEBUG_ID variable with given id
#define GFX_SET_ID_SETUP(id)
#endif

// Sets debug name for GPU object with given id and creates GFX_DEBUG_ID variable
// Before using needs call to GFX_ENABLE_ALL() or GFX_ENABLE_INFO()
#define GFX_SET_ID(child, id) GFX_SET_ID_SETUP(id); GFX_SET_ID_EX(child)

// Sets debug name for GPU object with generated RID and creates GFX_DEBUG_ID variable
// Before using needs call to GFX_ENABLE_ALL() or GFX_ENABLE_INFO()
#define GFX_SET_RID(child) GFX_SET_ID(child, GetRID())