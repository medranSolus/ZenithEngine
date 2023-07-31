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