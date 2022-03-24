#pragma once
#include "GFX/Resource/State.h"
#include "BarrierType.h"

namespace ZE::GFX::Pipeline
{
	// Descriptor containing params for performing resource transition
	struct TransitionDesc
	{
		RID RID;
		BarrierType Barrier;
		Resource::State BeforeState;
		Resource::State AfterState;
	};

	// Single info what transition barrier to perform on resource
	struct TransitionInfo
	{
		RID RID;
		Resource::State BeforeState;
		Resource::State AfterState;
	};
}