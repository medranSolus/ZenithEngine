#pragma once
#include "Types.h"

namespace ZE::GFX::Resource
{
	// Type of performed resource barrier
	enum class BarrierType : U8
	{
		Immediate,
		Begin,
		End
	};
}