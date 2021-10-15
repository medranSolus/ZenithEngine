#pragma once
#include "Types.h"

namespace ZE::GFX::Pipeline
{
	// Sync direction between possible GPU engines
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
		CopyToCompute
	};
}