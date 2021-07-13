#pragma once
#include "Types.h"

namespace ZE::GFX::API
{
	// Possible supported graphics APIs
	enum class ApiType : U8 { DX11, DX12, OpenGL, Vulkan };
}
namespace ZE
{
	typedef GFX::API::ApiType GfxApiType;
}