#pragma once
#include "Types.h"

namespace ZE::RHI
{
	// Possible supported graphics APIs
	enum class ApiType : U8 { DX11, DX12, OpenGL, Vulkan };
}
namespace ZE
{
	// Possible supported graphics APIs
	typedef RHI::ApiType GfxApiType;
}

#if _ZE_RHI_DX12
// Due to constraints in linking static libraries, include this define in one of your source files
// (easiest place is with main function), otherwise not all features of DirectX 12 will be available and device creation will probably fail
#	define ZE_ENABLE_AGILITYSDK \
	extern "C" \
	{ \
		ZE_EXPORT extern const UINT D3D12SDKVersion = 610; \
		ZE_EXPORT extern const char* D3D12SDKPath = ".\\AgilitySDK\\"; \
	}
#else
// Due to constraints in linking static libraries, include this define in one of your source files (easiest place is with main function),
// otherwise not all features of DirectX 12 will be available and device creation will probably fail
#	define ZE_ENABLE_AGILITYSDK
#endif