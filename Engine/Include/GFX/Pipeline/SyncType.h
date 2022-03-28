#pragma once
#include "Types.h"

namespace ZE::GFX::Pipeline
{
	// Sync direction between possible GPU engines
	// Naming QxToQy: queue Y needs to wait for queue X
	enum class SyncType : U8
	{
		None,

		MainToAll,
		MainToCompute,
		MainToCopy,

		ComputeToAll,
		ComputeToMain,
		ComputeToCopy,

		CopyToAll,
		CopyToMain,
		CopyToCompute,

		AllToMain,
		AllToCompute,
		AllToCopy
	};
}