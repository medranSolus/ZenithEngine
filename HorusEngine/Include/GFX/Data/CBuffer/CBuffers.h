#pragma once
#include "Types.h"

namespace GFX::Data::CBuffer
{
	struct Transform
	{
		Matrix transform;
		Matrix transformViewProjection;
	};
}