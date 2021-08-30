#pragma once
#include "Types.h"

namespace ZE::GFX
{
	// Types of commands that CommandList can receive
	enum class CommandType : U8 { All, Bundle, Compute, Copy };
}