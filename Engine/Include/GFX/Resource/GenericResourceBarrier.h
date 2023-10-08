#pragma once
#include "Generic.h"

namespace ZE::GFX::Resource
{
	// Barrier info to be performed on Generic resource
	struct GenericResourceBarrier
	{
		bool IsUAV;
		Generic* Resource;
		State Before;
		State After;
	};
}