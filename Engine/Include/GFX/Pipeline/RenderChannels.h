#pragma once
#include "Types.h"

namespace ZE::GFX::Pipeline
{
	enum Channel : U64
	{
		Main = 1,
		Shadow = 2,
		Light = 4,
		Depth = 8,
		All = -1
	};
}

namespace ZE
{
	typedef GFX::Pipeline::Channel RenderChannel;
}