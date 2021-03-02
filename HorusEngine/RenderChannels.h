#pragma once
#include <cstdint>

namespace GFX::Pipeline
{
	enum Channel : uint64_t
	{
		Main = 1,
		Shadow = 2,
		Light = 4,
		Depth = 8,
		All = -1
	};
}

typedef GFX::Pipeline::Channel RenderChannel;