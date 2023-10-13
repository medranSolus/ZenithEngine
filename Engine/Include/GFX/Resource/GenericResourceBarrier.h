#pragma once
#include "State.h"

namespace ZE::GFX::Resource
{
	// Barrier info to be performed on Generic resource
	struct GenericResourceBarrier
	{
		bool IsUAV;
		class Generic* Resource;
		State Before;
		State After;
	};
}