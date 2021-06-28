#pragma once
#include "Types.h"

namespace ZE::GFX::Data::CBuffer
{
	struct Transform
	{
		Matrix transform;
		Matrix transformViewProjection;
	};
}