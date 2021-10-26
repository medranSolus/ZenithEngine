#pragma once
#include "Types.h"

namespace ZE::GFX::Pipeline
{
	// Type of performed resource barrier
	enum class BarrierType : U8
	{
		Immediate,
		Begin,
		End
	};
}