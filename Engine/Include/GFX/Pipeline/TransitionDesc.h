#pragma once
#include "GFX/Resource/State.h"
#include "BarrierType.h"

namespace ZE::GFX::Pipeline
{
	// Descriptor containing params for performing resource transition
	struct TransitionDesc
	{
		U64 RID;
		BarrierType Barrier;
		Resource::State BeforeState;
		Resource::State AfterState;
	};
}